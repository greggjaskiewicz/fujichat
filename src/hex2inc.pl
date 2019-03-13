#!/usr/bin/perl -w

$|++;

while(<>) {
	chomp;
	my ($c, $data) = split /:/;
	$c = hex($c);
	next unless $c >= 32 && $c <= 127;

	my @data = ($data =~ /(..)/g);
	die "invalid data" unless @data == 6;
	my $last = pop @data;
	die "bottom row not blank: $last" if $last ne "00";

	$glyphs[$c] = \@data;
}

$idx = 0;
for(my $c = 32; $c < 128; $c += 2) {
	print " byte ";
	$bits = "";
	for(my $byte = 0; $byte < 5; ++$byte) {
		printf '$%02X', hex($glyphs[$c][$byte]) | (hex($glyphs[$c+1][$byte]) >> 4);
		print ", " unless $byte == 4;
		$bits .= sprintf "%08b\n", hex($glyphs[$c][$byte]) | (hex($glyphs[$c+1][$byte]) >> 4);
	}
	print " ; [$idx] $c," . ($c+1) . "  " . chr($c) . "," . chr($c+1) . "\n";
#	print STDERR $bits;
	$idx += 5;
}
