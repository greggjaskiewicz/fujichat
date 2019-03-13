#!/usr/bin/perl -w

use bytes;

$name = "xbm";
$width = 8;
$height = 384;
#$height = 512;
$cwidth = $width / 8;
$cheight = $height / 8;

print <<EOF;
#define ${name}_width ${width}
#define ${name}_height ${height}
static unsigned char ${name}_bits[] = {
EOF

undef $/;
$_ = <>;
@inbytes = split "";

# reverse bits, print 12 bytes/line

$c = 0;
for($i=0; $i<@inbytes; $i++) {
	$byte = ord $inbytes[$i];
	if(!$c) {
		print "   ";
	}

	printf "0x%02x", reverse_bits($byte);
	if($i != $#inbytes) {
		if($c == 12) {
			print ",\n";
			$c = 0;
		} else {
			print ", ";
			$c++;
		}
	}
}

print " };\n";

sub reverse_bits {
	my $bitstr = reverse sprintf("%08b", $_[0]);
	return eval "0b$bitstr";
}
