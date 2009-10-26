#!/usr/bin/env perl
use strict;

use Perl6::Slurp;
use YAML qw(Dump);
use WWW::Mechanize;

my %map = (
    'a' => \&hash,
    's' => \&string,
    'i' => \&integral,
    'b' => \&integral,
    'N' => \&null,
    ';' => \&swallow,
);

#my $within = 0;
my $text   = slurp;
my $pos    = 0;
#my $len    = length $text;

my $data = dispatch($text, \$pos);
print Dump $data;

sub dispatch {
    return unless length($_[0]);
    my $char = substr($_[0],${ $_[1] },1);
    return if $char eq '}';
    return $map{$char} ? $map{$char}->($_[0], $_[1]) : ();
}

sub hash {
    my ($pre, undef, $len, $val) = substr($_[0], ${ $_[1] }) =~ /^((a):(\d+):\{)(.*)/;
    my @list;
    my $here = 0;
    my $old;

    do {
        $old = $here;
        my @temp = dispatch($val, \$here);
        push @list, @temp;
    } while ($here > $old);

    ${ $_[1] } += length($pre) + $here + 1; # 1 for the closing brace
    return +{ @list };
}

sub string {
    my ($whole, undef, $len, $val) = substr($_[0], ${ $_[1] }) =~ /^((s):(\d+):"(.*?)")/;
    ${ $_[1] } += length $whole;
    return $val;
}

sub integral {
    my ($whole, undef, $val) = substr($_[0], ${ $_[1] }) =~ /^(([bi]):(\d+))/;
    ${ $_[1] } += length $whole;
    return $val;
}

sub null {
    ${ $_[1] }++;
    return undef;
}

sub swallow {
    ${ $_[1] } += 1;
    return ();
}

