from distutils.core import setup, Extension

setup(
	name="pysais-utf8",
	version="0.1",
	description="C binding for sais-lite to create suffix, LCP and BWT arrays",
	author="Lars Nieradzik",
	author_email="l.nieradzik@gmail.com",
	license="MIT",
	platforms="Any",
	classifiers=["Intended Audience :: Developers",
				 "License :: OSI Approved :: MIT License",
				 "Operating System :: OS Independent",
   			 	 "Programming Language :: Python",
    			 "Programming Language :: Python :: 3",
    			 "Programming Language :: Python :: 3.4",
    			 "Programming Language :: Python :: 3.5",
    			 "Programming Language :: Python :: 3.6",
    			 "Programming Language :: Python :: 3.7",
    			 "Topic :: Software Development :: Libraries :: Python Modules"],
    ext_modules=[Extension("sais",
    					   sources=["pysais.c", "sais.c"],
    					   extra_compile_args=["-O3", "-fomit-frame-pointer"])]
)