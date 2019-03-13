#!/usr/bin/perl -w

for(0..63) {
	$pix = $_*5;
	$byte = int($pix/8);
	$mask = 0xf800;
	$offs = $pix % 8;
	$mask >>= $offs;
	$mask = (~$mask) & 0xffff;
	$mleft = sprintf "%02X", $mask >> 8;
	$mright = sprintf "%02X", $mask & 0xff;
	$m = sprintf("%016b", $mask);
	printf "\$%02X, ", $offs;
#	print <<EOF;
#$_  $pix  $byte  $offs $mleft $mright $m
#EOF
}
