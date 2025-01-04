#!/bin/sh
echo ""
echo "Play Audio ..."
echo ""
CARD="sysdefault:CARD=ES8388"
aplay -D $CARD /opt/hardware/test/multimedias/AudioTest.wav

echo ""
echo "###################################################"
echo "# play audio from external device first           #"
echo "# Press <ENTER> to record 18s audio and play it...#"
echo "###################################################"
read junk

echo ""
echo "Record and Play recorded audio..."
echo ""
if [ -f /tmp/test.wav ]; then
        rm /tmp/test.wav
fi
arecord -D $CARD -f cd -V stereo -d 18 /tmp/test.wav && sleep 1 && aplay -D $CARD /tmp/test.wav
