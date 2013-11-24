#include "printutils.h"
#include "polyset.h"
#include "dxfdata.h"
#include "dxftess.h"
#include "CGAL_Nef_polyhedron.h"
#include "cgal.h"
#include "cgalutils.h"

CGAL_Nef_polyhedron::CGAL_Nef_polyhedron(CGAL_Nef_polyhedron2 *p)
{
	if (p) {
		dim = 2;
		p2.reset(p);
	}
	else {
		dim = 0;
	}
}

CGAL_Nef_polyhedron::CGAL_Nef_polyhedron(CGAL_Nef_polyhedron3 *p)
{
	if (p) {
		dim = 3;
		p3.reset(p);
	}
	else {
		dim = 0;
	}
}


CGAL_Nef_polyhedron& CGAL_Nef_polyhedron::operator+=(const CGAL_Nef_polyhedron &other)
{
	if (this->dim == 2) (*this->p2) += (*other.p2);
	else if (this->dim == 3) (*this->p3) += (*other.p3);
	return *this;
}

CGAL_Nef_polyhedron& CGAL_Nef_polyhedron::operator*=(const CGAL_Nef_polyhedron &other)
{
	if (this->dim == 2) (*this->p2) *= (*other.p2);
	else if (this->dim == 3) (*this->p3) *= (*other.p3);
	return *this;
}

CGAL_Nef_polyhedron& CGAL_Nef_polyhedron::operator-=(const CGAL_Nef_polyhedron &other)
{
	if (this->dim == 2) (*this->p2) -= (*other.p2);
	else if (this->dim == 3) (*this->p3) -= (*other.p3);
	return *this;
}

extern CGAL_Nef_polyhedron2 minkowski2(const CGAL_Nef_polyhedron2 &a, const CGAL_Nef_polyhedron2 &b);

CGAL_Nef_polyhedron &CGAL_Nef_polyhedron::minkowski(const CGAL_Nef_polyhedron &other)
{
	if (this->dim == 2) (*this->p2) = minkowski2(*this->p2, *other.p2);
	else if (this->dim == 3) (*this->p3) = CGAL::minkowski_sum_3(*this->p3, *other.p3);
	return *this;
}

int CGAL_Nef_polyhedron::weight() const
{
	if (this->isNull()) return 0;

	size_t memsize = sizeof(CGAL_Nef_polyhedron);
	if (this->dim == 2) {
		memsize += sizeof(CGAL_Nef_polyhedron2) +
			this->p2->explorer().number_of_vertices() * sizeof(CGAL_Nef_polyhedron2::Explorer::Vertex) +
			this->p2->explorer().number_of_halfedges() * sizeof(CGAL_Nef_polyhedron2::Explorer::Halfedge) +
			this->p2->explorer().number_of_edges() * sizeof(CGAL_Nef_polyhedron2::Explorer::Face);
	}
	if (this->dim == 3) memsize += this->p3->bytes();
	return memsize;
}

/*!
	Creates a new PolySet and initializes it with the data from this polyhedron

	This method is not const since convert_to_Polyhedron() wasn't const
	in earlier versions of CGAL.

	Note: Can return NULL if an error occurred
*/
PolySet *CGAL_Nef_polyhedron::convertToPolyset()
{
	PRINTD("Nef Poly3: convert to polyset.");
	if (this->isNull()) return new PolySet();
	PolySet *ps = new PolySet();
	if (this->dim == 2) {
		DxfData *dd = this->convertToDxfData();
		ps->is2d = true;
		dxf_tesselate(ps, *dd, 0, Vector2d(1,1), true, false, 0);
		dxf_border_to_ps(ps, *dd);
		delete dd;
	}
	else if (this->dim == 3) {
		CGAL::Failure_behaviour old_behaviour = CGAL::set_error_behaviour(CGAL::THROW_EXCEPTION);
		CGAL_Polyhedron P;
		try {
			//nef3_to_polyhedron( *(this->p3), P, OpenSCAD::facetess::EARCLIP, OpenSCAD::facetess::EARCLIP );
			PRINTDB("Nef: Valid: %i Simple: %i",this->p3->is_valid()%this->p3->is_simple());
			bool err = CGAL::nefhelper_convert_to_polyhedron<CGAL_Polyhedron,CGAL_Kernel3,CGAL_Nef_polyhedron3>( *(this->p3), P );
			PRINTDB("Nef->Polyhedron Conversion. Vertices: %i Faces: %i",P.size_of_vertices()%P.size_of_facets());
			if (err) PRINT("WARNING: Nef->Polyhedron error. Mesh corrupted");
			bool ok = createPolySetFromPolyhedron(P, *ps);
			if (!ok) { delete ps; ps=NULL; }
		}
		catch (const CGAL::Precondition_exception &e) {
			PRINTB("CGAL Precondition error in CGAL_Nef_polyhedron::convertToPolyset(): %s", e.what());
		}
		catch (const CGAL::Assertion_exception *e) {
			PRINTB("CGAL Assertion error in CGAL_Nef_polyhedron::convertToPolyset(): %s", e->what());
		}
		catch (...) {
			PRINT("Unknown CGAL error converting Nef to Polyset.");
		}
		CGAL::set_error_behaviour(old_behaviour);
	}
	PRINTD("Nef Poly3: convert to polyset. end");
	return ps;
}

/*!
	Deep copy
*/
CGAL_Nef_polyhedron CGAL_Nef_polyhedron::copy() const
{
	CGAL_Nef_polyhedron copy = *this;
	if (copy.p2) copy.p2.reset(new CGAL_Nef_polyhedron2(*copy.p2));
	else if (copy.p3) copy.p3.reset(new CGAL_Nef_polyhedron3(*copy.p3));
	return copy;
}


