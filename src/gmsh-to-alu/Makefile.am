noinst_HEADERS =				\
	main.hh

headercheck_IGNORE =				\
	main.hh

bin_PROGRAMS =

if ALUGRID
bin_PROGRAMS += dune-gmsh-to-alusimplex-3d
bin_PROGRAMS += dune-gmsh-to-alucube-3d
dune_gmsh_to_alusimplex_3d_SOURCES = alusimplex-3d.cc
dune_gmsh_to_alusimplex_3d_CPPFLAGS = $(AM_CPPFLAGS)	\
	$(ALUGRID_CPPFLAGS)
dune_gmsh_to_alusimplex_3d_LDFLAGS = $(AM_LDFLAGS)	\
	$(ALUGRID_LDFLAGS)
dune_gmsh_to_alusimplex_3d_LDADD =		\
	$(ALUGRID_LIBS)				\
	$(LDADD)
dune_gmsh_to_alucube_3d_SOURCES = alucube-3d.cc
dune_gmsh_to_alucube_3d_CPPFLAGS = $(AM_CPPFLAGS)	\
	$(ALUGRID_CPPFLAGS)
dune_gmsh_to_alucube_3d_LDFLAGS = $(AM_LDFLAGS)	\
	$(ALUGRID_LDFLAGS)
dune_gmsh_to_alucube_3d_LDADD =		\
	$(ALUGRID_LIBS)				\
	$(LDADD)
endif ALUGRID

include $(top_srcdir)/am/global-rules

EXTRA_DIST = CMakeLists.txt
