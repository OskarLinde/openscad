#ifndef DXFLINEXTRUDENODE_H_
#define DXFLINEXTRUDENODE_H_

#include "node.h"
#include "visitor.h"

class DxfLinearExtrudeNode : public AbstractPolyNode
{
public:
	DxfLinearExtrudeNode(const ModuleInstantiation *mi) : AbstractPolyNode(mi) {
		convexity = slices = 0;
		fn = fs = fa = height = twist = 0;
		origin_x = origin_y = scale = 0;
		center = has_twist = false;
	}
  virtual Response accept(class State &state, Visitor &visitor) const {
		return visitor.visit(state, *this);
	}
	virtual std::string toString() const;
	virtual std::string name() const { return "linear_extrude"; }

	int convexity, slices;
	double fn, fs, fa, height, twist;
	double origin_x, origin_y, scale;
	bool center, has_twist;
	QString filename, layername;
	virtual PolySet *evaluate_polyset(render_mode_e mode, class PolySetEvaluator *) const;
};

#endif