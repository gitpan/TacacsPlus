use ExtUtils::MakeMaker;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.
WriteMakefile(
    'NAME'	=> 'Authen::TacacsPlus',
    'VERSION_FROM' => 'TacacsPlus.pm', # finds $VERSION
    'LIBS'	=> [''],   # e.g., '-lm' 
    'DEFINE'	=> '',     # e.g., '-DHAVE_SOMETHING' 
    'INC'	=> '',     # e.g., '-I/usr/include/other' 
    'MYEXTLIB' => 'tacpluslib/libtacplus$(LIB_EXT)',
    'DISTNAME'          => 'TacacsPlus',
    'dist'                  => { COMPRESS => 'gzip', SUFFIX => 'gz' },
);
sub MY::postamble {         
'                           
$(MYEXTLIB): tacpluslib/Makefile 
	cd tacpluslib && $(MAKE) 
';                          
}                           
