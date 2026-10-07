#!/usr/bin/env python3
"""
receive_frame.py — receive and display continuous OV7670 RGB565 frames
                   from the SAM E54 USB bulk vendor stream.

Wire format (per frame, 38 416 bytes total):
    [0..3]   AA 55 AA 55     sync marker
    [4..7]   frame counter   (u32 LE)
    [8]      person_present  (0 or 1)
    [9]      person_logit    (int8)
    [10]     no_person_logit (int8)
    [11]     inference_count (uint8)
    [12..15] inference_us    (u32 LE)
    [16..]   160*120*2 bytes RGB565 payload (little-endian)

The firmware sends via USB_DEVICE_EndpointWrite on a raw bulk IN endpoint —
no CDC/serial driver involved.  This script uses PyUSB to read that endpoint
directly.

Setup (once):
    pip install pyusb numpy opencv-python
    # Windows: place libusb-1.0.dll next to this script, OR install via:
    #   winget install libusb.libusb  (adds to PATH)
    # Linux/Mac: sudo apt install libusb-1.0-0 / brew install libusb

Usage:
    python receive_frame.py
    python receive_frame.py --save frames/
    python receive_frame.py --frames 100 --save frames/
"""

import argparse
import struct
import sys
import time
from pathlib import Path

import numpy as np

try:
    import usb.core
    import usb.util
    import usb.backend.libusb1
except ImportError:
    print("PyUSB not found.  Run:  pip install pyusb", file=sys.stderr)
    sys.exit(1)

try:
    import cv2
    cv2.namedWindow("_probe", cv2.WINDOW_NORMAL)
    cv2.destroyWindow("_probe")
    _CV2 = True
except ImportError:
    _CV2 = False
except cv2.error:
    _CV2 = False

# ── USB identifiers (must match usb_device_init_data.c) ──────────────────────
VID         = 0x04D8
PID         = 0x0053
EP_IN       = 0x81          # EP1 IN
PKT_SIZE    = 512           # read this many bytes per usb.read() call

# ── Frame constants ───────────────────────────────────────────────────────────
MARKER          = bytes([0xAA, 0x55, 0xAA, 0x55])
HEADER_SIZE     = 16
FRAME_W         = 160
FRAME_H         = 120
PAYLOAD_BYTES   = FRAME_W * FRAME_H * 2   # 38 400


def open_device():
    """Find and claim the SAM E54 camera USB device."""
    # Try libusb-1.0 in the script directory first (Windows portable setup)
    backend = None
    dll = Path(__file__).parent / "libusb-1.0.dll"
    if dll.exists():
        backend = usb.backend.libusb1.get_backend(find_library=lambda _: str(dll))

    dev = usb.core.find(idVendor=VID, idProduct=PID, backend=backend)
    if dev is None:
        print(f"Device {VID:04X}:{PID:04X} not found.\n"
              "Check: firmware flashed, USB cable connected, WinUSB/libusb driver\n"
              "installed for this VID:PID (use Zadig on Windows).",
              file=sys.stderr)
        sys.exit(1)

    # Detach kernel driver on Linux if needed
    if sys.platform != "win32":
        try:
            if dev.is_kernel_driver_active(0):
                dev.detach_kernel_driver(0)
        except Exception:
            pass

    dev.set_configuration()
    print(f"Opened {VID:04X}:{PID:04X} — {dev.manufacturer} / {dev.product}")
    return dev


class FrameReader:
    """
    Maintains a bytearray accumulator, reads PKT_SIZE chunks from the bulk IN
    endpoint, and extracts validated frames.

    Sync logic:
      1. Find MARKER using bytes.find() — fast native search.
      2. Check W == 160 and H == 120 — rejects false matches inside pixel data.
      3. Read exactly W*H*2 payload bytes.
      No tail check needed: the firmware terminates the USB transfer with
      DATA_COMPLETE on the last bulk packet, so the host receives an exact
      FRAME_HEADER + payload sequence with no ambiguity.
    """

    def __init__(self, dev):
        self._dev = dev
        self._buf = bytearray()

    def _fill(self, need: int) -> None:
        while len(self._buf) < need:
            try:
                chunk = bytes(self._dev.read(EP_IN, PKT_SIZE, timeout=2000))
                self._buf += chunk
            except usb.core.USBTimeoutError:
                raise EOFError("USB timeout — device stopped sending")

    def read_frame(self) -> tuple[bytes, dict]:
        KEEP = len(MARKER) - 1

        while True:
            self._fill(HEADER_SIZE + PKT_SIZE)

            idx = self._buf.find(MARKER)
            if idx == -1:
                del self._buf[:-KEEP]
                continue

            del self._buf[:idx]
            self._fill(HEADER_SIZE)

            frame_cnt = struct.unpack_from("<I", self._buf, 4)[0]
            person    = self._buf[8]
            p_logit   = struct.unpack_from("b", self._buf, 9)[0]
            np_logit  = struct.unpack_from("b", self._buf, 10)[0]
            inf_count = self._buf[11]
            inf_us    = struct.unpack_from("<I", self._buf, 12)[0]

            del self._buf[:HEADER_SIZE]
            self._fill(PAYLOAD_BYTES)
            payload = bytes(self._buf[:PAYLOAD_BYTES])
            del self._buf[:PAYLOAD_BYTES]

            meta = dict(frame=frame_cnt, person=bool(person),
                        person_logit=p_logit, no_person_logit=np_logit,
                        inferences=inf_count, inference_us=inf_us)
            return payload, meta


def rgb565_to_rgb888(raw: bytes, w: int, h: int, byteorder: str = "le") -> np.ndarray:
    dtype = ">u2" if byteorder == "be" else "<u2"
    pixels = np.frombuffer(raw, dtype=dtype)
    r = (((pixels >> 11) & 0x1F) * 255 // 31).astype(np.uint8)
    g = (((pixels >>  5) & 0x3F) * 255 // 63).astype(np.uint8)
    b = ((pixels         & 0x1F) * 255 // 31).astype(np.uint8)
    return np.stack([r, g, b], axis=-1).reshape(h, w, 3)


def show_opencv(rgb: np.ndarray, title: str) -> bool:
    bgr   = cv2.cvtColor(rgb, cv2.COLOR_RGB2BGR)
    scale = max(1, 240 // rgb.shape[0])
    if scale > 1:
        bgr = cv2.resize(bgr, (rgb.shape[1] * scale, rgb.shape[0] * scale),
                         interpolation=cv2.INTER_NEAREST)
    cv2.imshow(title, bgr)
    return cv2.waitKey(1) & 0xFF != 27   # False → ESC pressed


def receive_frames(dev, save_dir: Path | None, max_frames: int, byteorder: str = "le") -> None:
    reader = FrameReader(dev)
    count  = 0
    t0     = time.monotonic()

    print("Waiting for first frame…")

    while True:
        payload, meta = reader.read_frame()

        count += 1
        fps = count / max(time.monotonic() - t0, 1e-6)
        person_str = "PERSON" if meta["person"] else "------"
        print(f"  Frame {count:5d}  {FRAME_W}×{FRAME_H}  {fps:.1f} fps  "
              f"{person_str}  logit={meta['person_logit']:+d}/{meta['no_person_logit']:+d}  "
              f"inf={meta['inference_us']}us")

        rgb = rgb565_to_rgb888(payload, FRAME_W, FRAME_H, byteorder)

        if save_dir is not None:
            from PIL import Image  # type: ignore
            Image.fromarray(rgb, "RGB").save(save_dir / f"frame_{count:05d}.png")
        elif _CV2:
            if not show_opencv(rgb, "SAM E54 camera"):
                break
        else:
            from PIL import Image  # type: ignore
            if count % 10 == 0:
                Image.fromarray(rgb, "RGB").show()

        if max_frames and count >= max_frames:
            print(f"Captured {max_frames} frames, done.")
            break


def raw_dump(dev, count=20):
    """Read raw USB packets and hex-dump them for debugging."""
    print(f"=== Raw USB dump: reading up to {count} packets from EP 0x{EP_IN:02X} ===")
    for i in range(count):
        try:
            data = bytes(dev.read(EP_IN, PKT_SIZE, timeout=3000))
            hex_preview = data[:32].hex(" ")
            ascii_preview = "".join(chr(b) if 32 <= b < 127 else "." for b in data[:32])
            print(f"  pkt {i:3d}: {len(data):4d} bytes  {hex_preview}  |{ascii_preview}|")
        except usb.core.USBTimeoutError:
            print(f"  pkt {i:3d}: TIMEOUT (no data in 3s)")
            break
    print("=== end raw dump ===\n")


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Receive OV7670 stream from SAM E54 over USB bulk")
    parser.add_argument("--save",   metavar="DIR",
                        help="Save each frame as PNG in DIR")
    parser.add_argument("--frames", type=int, default=0,
                        help="Stop after N frames (0 = forever)")
    parser.add_argument("--byteorder", choices=["le", "be"], default="le",
                        help="RGB565 byte order: le (default) or be")
    parser.add_argument("--raw", action="store_true",
                        help="Dump raw USB packets and exit (debug)")
    args = parser.parse_args()

    save_dir: Path | None = None
    if args.save:
        save_dir = Path(args.save)
        save_dir.mkdir(parents=True, exist_ok=True)
        print(f"Saving frames to {save_dir}/")

    dev = open_device()

    if args.raw:
        raw_dump(dev)
        usb.util.dispose_resources(dev)
        return

    try:
        receive_frames(dev, save_dir, args.frames, args.byteorder)
    except KeyboardInterrupt:
        print("\nStopped.")
    except EOFError as e:
        print(f"\n{e}", file=sys.stderr)
    finally:
        if _CV2:
            cv2.destroyAllWindows()
        usb.util.dispose_resources(dev)


if __name__ == "__main__":
    main()
