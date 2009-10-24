#use 5.008008;
use ExtUtils::MakeMaker;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.
WriteMakefile(
    NAME              => 'PHP::Serialization::XS',
    VERSION_FROM      => 'lib/PHP/Serialization/XS.pm', # finds $VERSION
    PREREQ_PM         => { 'PHP::Serialization' => '0.33' }, # e.g., Module::Name => 1.1
    ($] >= 5.005 ?     ## Add these new keywords supported since 5.005
      (ABSTRACT_FROM  => 'lib/PHP/Serialization/XS.pm', # retrieve abstract from module
       AUTHOR         => 'Darren Kulp <kulp@cpan.org>') : ()),
    LIBS              => ['-L. -lphpserialize -lyaml'],
    DEFINE            => '', # e.g., '-DHAVE_SOMETHING'
    INC               => '-I.', # e.g., '-I. -I/usr/include/other'
    OBJECT            => '$(O_FILES)', # link all the C files too
    CCFLAGS           => '-std=c99 -D_XOPEN_SOURCE=700',
);
