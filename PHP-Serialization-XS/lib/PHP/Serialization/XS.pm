package PHP::Serialization::XS;

use strict;
use warnings;
use bytes;

use PHP::Serialization qw(serialize);

require Exporter;

our @ISA = qw(PHP::Serialization Exporter);

our %EXPORT_TAGS = (all => [ qw(serialize unserialize) ]);
our @EXPORT_OK = @{ $EXPORT_TAGS{all} };

our $VERSION = '0.01';

require XSLoader;
XSLoader::load('PHP::Serialization::XS', $VERSION);

sub unserialize {
    my ($str, $class) = @_;
    return _c_decode($str || "", $class);
}

sub decode {
    my ($self, $str, $class) = @_;
    return unserialize($str, $class);
}

1;
__END__

=head1 NAME

PHP::Serialization::XS - simple flexible means of converting the output
of PHP's serialize() into the equivalent Perl memory structure, and vice
versa - XS version.

=head1 SYNOPSIS

    use PHP::Serialization:XS qw(serialize unserialize);
    my $encoded = serialize({ a => 1, b => 2 });
    my $hashref = unserialize($encoded);

See L<PHP::Serialization>.

=head1 DESCRIPTION

This module provides the same interface as L<PHP::Serialization>, but
uses XS during deserialization, for speed enhancement.

If you have code written for C<PHP::Serialization>, you should be able to
replace all references to C<PHP::Serialization> with
C<PHP::Serialization::XS> and notice no change except for an increase in
speed of deserialization.

Node that serialization is still provided by C<PHP::Serialization>, and
its speed should therefore not be affected. This is why
C<PHP::Serialization::XS> requires C<PHP::Serialization> to be
installed.

=head1 TODO

More tests.

=head1 SEE ALSO

L<PHP::Serialization>

=head1 AUTHOR

Darren Kulp, E<lt>kulp@cpan.orgE<gt>

Tests stolen shamelessly from Tomas Doran's L<PHP::Serialization> package.

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2009 by Darren Kulp

This library is free software; you can redistribute it and/or modify
it under the same terms as Perl itself, either Perl version 5.10.0 or,
at your option, any later version of Perl 5 you may have available.

=cut

