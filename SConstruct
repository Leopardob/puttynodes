import os, sys, vfx.build

user = vfx.build.execute('whoami')['output']
host = vfx.build.execute('hostname')['output']

project = "puttyNodes"
version = "0.1.0"

# checks
maya_location = os.getenv('MAYA_LOCATION')
if maya_location == None:
	vfx.build.errormsg( "The Maya environment not set!")
	
	
# includes/srcs
mayaplugin_cpppath = [ '#/include', os.path.join( maya_location, 'include' ), '/usr/X11R6/include' ]

mayaplugin_cppsrcs = [  'src/plugin.cpp', 
                        'src/puttyDeformerNode.cpp',  
                        'src/puttyFieldNode.cpp',                          
                        'src/puttyParticleAttributeMapperNode.cpp',                                                  
                        ]

# libs
mayaplugin_libpath = [ os.path.join( maya_location, 'lib' ) ]
#mayaplugin_libs = [ 'pthread', 'OpenMaya', 'OpenMayaRender', 'OpenMayaAnim', 'OpenMayaUI', 'base' ]
mayaplugin_libs = [ 'pthread', 'OpenMaya', 'OpenMayaRender', 'OpenMayaAnim', 'OpenMayaUI', 'OpenMayaFX' ]

# compilation flags
mayaplugin_cflags = [ '-O2', '-pthread', '-pipe', '-mcpu=pentium4' ]
mayaplugin_cppflags = mayaplugin_cflags + [ '-Wno-deprecated', '-fno-gnu-keywords', '--fast-math' ]
mayaplugin_defines = [ '_BOOL', 'LINUX', 'REQUIRE_IOSTREAM', '__USER__="'+user+'"', '__HOST__="'+host+'"', '__PROJECTNAME__="'+project+'"', '__BUILDVERSION__="'+version+'"' ]

# build environment
mayaplugin_env = Environment( 	CC = 'gcc',
							 	CXX = 'g++',
							 	CFLAGS = mayaplugin_cflags, 
							 	CXXFLAGS = mayaplugin_cppflags,
							 	CPPDEFINES = mayaplugin_defines,
								CPPPATH = mayaplugin_cpppath,
								LIBPATH = mayaplugin_libpath,
								LIBS = mayaplugin_libs,
								SHLIBPREFIX='' )
mayaplugin_env.SConsignFile()

# plugin		  
mayaplugin = mayaplugin_env.SharedLibrary( project, mayaplugin_cppsrcs )

# dist clean
mayaplugin_env.Clean( 'dist', [ '.sconsign.dblite' ] )
