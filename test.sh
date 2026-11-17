#!/usr/bin/env bash
# Smoke tests for socketcanDecodeSignal.
# Runs without a CAN bus: log lines are piped to stdin, as candump -L would.
# Expected values verified against cantools (python -m pip install cantools).
#
# Usage: ./test.sh          (builds the binary via make unless present)
#        ASAN=1 ./test.sh   (binary was built with sanitizers)

set -u
cd "$(dirname "$0")"

if [ ! -x ./socketcanDecodeSignal ]; then
	make --no-print-directory socketcanDecodeSignal
fi

pass=0
fail=0

# contains <desc> <pattern> <output>
contains() {
	if grep -qF -- "$2" <<<"$3"; then
		pass=$((pass + 1))
	else
		fail=$((fail + 1))
		echo "FAIL: $1"
		echo "  expected to contain: $2"
		echo "  got: $3"
	fi
}

# not_contains <desc> <pattern> <output>
not_contains() {
	if grep -qF -- "$2" <<<"$3"; then
		fail=$((fail + 1))
		echo "FAIL: $1"
		echo "  expected NOT to contain: $2"
		echo "  got: $3"
	else
		pass=$((pass + 1))
	fi
}

decode() {
	# decode <input-line> [frame[.signal] ...]
	local input="$1"
	shift
	printf '%s\n' "$input" | ./socketcanDecodeSignal ccl_test.dbc "$@" 2>/dev/null
}

MUX0=001#0000100100820100
MUX1=001#8d00100100820100
MUX2=001#0200100100820100
MUX3=001#0300100100820100
FRAME2=002#0C00057003CD1F83

# --- plain (non multiplexed) frame ---
out=$(decode "(0.0) vcan0 $FRAME2" testFrame2)
contains "testFrame2 decodes signals" "secSig4: 0x02 02" "$out"
contains "testFrame2 signed signal" "secSig11: 0xffffffffffffff70 -144" "$out"
contains "testFrame2 signal spanning two bytes" "secSig10: 0x500 1280" "$out"

# --- single signal subscription ---
out=$(decode "(0.0) vcan0 $FRAME2" testFrame2.secSig10)
contains "single signal output" "secSig10: 0x500 1280" "$out"
not_contains "single signal: no other signals" "secSig11:" "$out"

# --- multiplexed frame, muxer value 1 ---
out=$(decode "(0.0) vcan0 $MUX1" testFrame1)
contains "mux: muxer value reported" "sig0: 0x01 01" "$out"
contains "mux: group m1 decoded" "sig1: 0x23 35" "$out"
not_contains "mux: group m2 not decoded" "sig3:" "$out"
not_contains "mux: muxer signal not repeated" "sig0: 0x02" "$out"

# --- multiplexed frame, muxer value 2 ---
out=$(decode "(0.0) vcan0 $MUX2" testFrame1)
contains "mux2: group m2 decoded" "sig3: 0x800 2048" "$out"
contains "mux2: second m2 signal" "sig4: 0x100 256" "$out"
not_contains "mux2: group m1 not decoded" "sig1: 0x23" "$out"

# --- multiplexed frame, muxer value 3 (no group defined) ---
out=$(decode "(0.0) vcan0 $MUX3" testFrame1)
contains "mux3: plain signals still decoded" "sig5: 0x01 01" "$out"
not_contains "mux3: no m-group decoded" "sig3:" "$out"

# --- multiplexed frame, single signal of wrong group ---
out=$(decode "(0.0) vcan0 $MUX1" testFrame1.sig3)
contains "subscribed wronggroup: muxer still reported" "sig0: 0x01" "$out"
not_contains "subscribed wronggroup: signal suppressed" "sig3:" "$out"

# --- malformed lines are skipped, exit stays 0 ---
printf '(garbage\nnosuchframe#ff\n(0.0) vcan0 %s\n' "$FRAME2" >/tmp/scds_test.log
out=$(./socketcanDecodeSignal ccl_test.dbc testFrame2.secSig4 </tmp/scds_test.log 2>/dev/null)
rc=$?
if [ "$rc" -eq 0 ]; then
	pass=$((pass + 1))
else
	fail=$((fail + 1))
	echo "FAIL: malformed input must exit with 0, got $rc"
fi
contains "malformed input skipped, frame still decoded" "secSig4: 0x02 02" "$out"

echo
echo "$pass passed, $fail failed"
[ "$fail" -eq 0 ]
