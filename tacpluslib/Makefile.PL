use ExtUtils::MakeMaker;                                         
$Verbose = 1;                                                    
WriteMakefile(                                                   
    NAME      => 'Authen::TacacsPlus::tacplus',                               
    SKIP      => [qw(all static static_lib dynamic dynamic_lib)],
    clean     => {'FILES' => 'libtacplus$(LIB_EXT)'},              
);                                                               
                                                                 
sub MY::top_targets {     

'
all:	static                                   
                                                 
static:	libtacplus$(LIB_EXT)              
                                                 
libtacplus$(LIB_EXT):	$(O_FILES)                  
	$(AR) cr libtacplus$(LIB_EXT) $(O_FILES)  
	$(RANLIB) libtacplus$(LIB_EXT)            
                                                 
 ';                                              
 }                                               

