librule(name="sparse",
		    headers=["sparse_matrix.h", "sparse_matrix_impl.h"],
#		            ]
deplibs=["trilinos:trilinos", "base:base"]
       );

binrule(name="mtest",
			  sources=["sparse_matrix_test.cc"],
				cflags=" -fexceptions",
				deplibs=[":sparse"]
			 );

