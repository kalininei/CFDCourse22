#include <fstream>
#include <sstream>
#include "grid/grid_builder.hpp"

namespace{

struct AVtkCell{
	AVtkCell(std::vector<int>&& p): points(std::move(p)) {}
	virtual ~AVtkCell() = default;

	// all faces normals are outward with respect to the cell
	std::vector<std::vector<int>> faces() const{
		std::vector<std::vector<int>> ret = faces_local();
		for (auto& it1: ret)
		for (auto& it2: it1){
			it2 = points[it2];
		}
		return ret;
	}
	virtual std::vector<std::vector<int>> faces_local() const = 0;

	virtual int dim() const = 0;
	virtual int code() const = 0;

	std::vector<int> points;
	static std::shared_ptr<AVtkCell> build(std::vector<int> points, int code);
};

struct VertexCell: public AVtkCell{
	VertexCell(std::vector<int>&& p): AVtkCell(std::move(p)) {}

	std::vector<std::vector<int>> faces_local() const override{
		throw std::runtime_error("Vertex cells has no faces");
	}

	int dim() const override { return 0; }
	int code() const override { return 1; }
};

struct LineCell: public AVtkCell{
	LineCell(std::vector<int>&& p): AVtkCell(std::move(p)) {}
	std::vector<std::vector<int>> faces_local() const override{
		return { {0}, {1} };
	}
	int dim() const override { return 1; }
	int code() const override { return 3; }
};

struct PolygonCell: public AVtkCell{
	PolygonCell(std::vector<int>&& p): AVtkCell(std::move(p)) {}
	std::vector<std::vector<int>> faces_local() const override{
		std::vector<std::vector<int>> ret;
		for (int i=0; i<(int)points.size(); ++i){
			int inext = (i==(int)points.size()-1) ? 0 : i+1;
			ret.push_back({i, inext});
		}
		return ret;
	}
	int dim() const override { return 2; }
	int code() const override { return 7; }
};

struct TetraCell: public AVtkCell{
	TetraCell(std::vector<int>&& p): AVtkCell(std::move(p)) {}
	std::vector<std::vector<int>> faces_local() const override{
		return {
			{0, 2, 1},
			{1, 2, 3},
			{0, 1, 3},
			{0, 3, 2}
		};
	}
	int dim() const override { return 3; }
	int code() const override { return 10; }
};

struct PyramidCell: public AVtkCell{
	PyramidCell(std::vector<int>&& p): AVtkCell(std::move(p)) {}
	std::vector<std::vector<int>> faces_local() const override{
		return {
			{0, 3, 2, 1},
			{0, 4, 3},
			{2, 3, 4},
			{1, 4, 0},
			{1, 2, 4}
		};
	}
	int dim() const override { return 3; }
	int code() const override { return 14; }
};

struct WedgeCell: public AVtkCell{
	WedgeCell(std::vector<int>&& p): AVtkCell(std::move(p)) {}
	std::vector<std::vector<int>> faces_local() const override{
		return {
			{0, 3, 4, 1},
			{3, 5, 4},
			{0, 1, 2},
			{1, 4, 5, 2},
			{0, 2, 5, 3}
		};
	}
	int dim() const override { return 3; }
	int code() const override { return 13; }
};

struct HexaCell: public AVtkCell{
	HexaCell(std::vector<int>&& p): AVtkCell(std::move(p)) {}
	std::vector<std::vector<int>> faces_local() const override{
		return {
			{0, 1, 5, 4},
			{3, 7, 6, 2},
			{1, 2, 6, 5},
			{0, 4, 7, 3},
			{0, 3, 2, 1},
			{4, 5, 6, 7}
		};
	}
	int dim() const override { return 3; }
	int code() const override { return 12; }
};

struct PentaPrismCell: public AVtkCell{
	PentaPrismCell(std::vector<int>&& p): AVtkCell(std::move(p)) {}
	std::vector<std::vector<int>> faces_local() const override{
		return {
			{0, 4, 3, 2, 1},
			{5, 6, 7, 8, 9},
			{0, 1, 6, 5},
			{1, 2, 7, 6},
			{2, 3, 8, 7},
			{3, 4, 9, 8},
			{4, 0, 5, 9}
		};
	}
	int dim() const override { return 3; }
	int code() const override { return 15; }
};

struct HexaPrismCell: public AVtkCell{
	HexaPrismCell(std::vector<int>&& p): AVtkCell(std::move(p)) {}

	std::vector<std::vector<int>> faces_local() const override{
		return {
			{0, 5, 4, 3, 2, 1},
			{6, 7, 8, 9, 10, 11},
			{0, 1, 7, 6},
			{1, 2, 8, 7},
			{2, 3, 9, 8},
			{3, 4, 10, 9},
			{4, 5, 11, 10},
			{5, 0, 6, 11}
		};
	}

	int dim() const override { return 3; }
	int code() const override { return 16; }
};

std::shared_ptr<AVtkCell> AVtkCell::build(std::vector<int> points, int code){
	if (code == 1){
		return std::make_shared<VertexCell>(std::move(points));
	} else if (code == 3){
		return std::make_shared<LineCell>(std::move(points));
	} else if (code == 5 || code == 7 || code == 9){
		return std::make_shared<PolygonCell>(std::move(points));
	} else if (code == 8){
		std::swap(points[2], points[3]);
		return std::make_shared<PolygonCell>(std::move(points));
	} else if (code == 10){
		return std::make_shared<TetraCell>(std::move(points));
	} else if (code == 12){
		return std::make_shared<HexaCell>(std::move(points));
	} else if (code == 11){
		std::swap(points[2], points[3]);
		std::swap(points[6], points[7]);
		return std::make_shared<HexaCell>(std::move(points));
	} else if (code == 13){
		return std::make_shared<WedgeCell>(std::move(points));
	} else if (code == 14){
		return std::make_shared<PyramidCell>(std::move(points));
	} else if (code == 15){
		return std::make_shared<PentaPrismCell>(std::move(points));
	} else if (code == 16){
		return std::make_shared<HexaPrismCell>(std::move(points));
	} else {
		throw std::runtime_error("Unsupported vtk cell type: " + std::to_string(code));
	}
}


struct VtkFace{
	static constexpr int MAX_FACE_LEN = 6;

	VtkFace(const std::vector<int>& vpoints, bool& was_reverted){
		if (vpoints.size() > MAX_FACE_LEN){
			throw std::runtime_error("Faces with more than " 
			                         + std::to_string(MAX_FACE_LEN) 
			                         + " are not supported");
		}
		was_reverted = false;
		points_size = vpoints.size();
		std::copy(vpoints.begin(), vpoints.end(), points.begin());
		std::fill(points.begin()+points_size, points.end(), -1);

		if (vpoints.size() == 2){
			if (points[0] > points[1]){
				std::swap(points[0], points[1]);
				was_reverted = true;
			}
		} else if (vpoints.size() > 2){
			auto minit = std::min_element(points.begin(), points.begin() + points_size);
			std::rotate(points.begin(), minit, points.begin() + points_size);
			if (points[1] > points[points_size-1]){
				was_reverted = true;
				std::reverse(points.begin()+1, points.begin()+points.size());
			}
		}
	};

	bool is_boundary() const{
		return ((left_cell<0 && right_cell>=0)
		       || (left_cell>=0 && right_cell<0));
	};

	std::vector<int> points_vec() const{
		return std::vector<int>(points.begin(), points.begin() + points_size);
	};

	std::array<int, MAX_FACE_LEN> points;
	int index = -1;
	int points_size = 0;
	int left_cell = -1;
	int right_cell = -1;

	friend bool operator<(const VtkFace& f1, const VtkFace& f2){
		return f1.points < f2.points;
	};
};


struct VtkFaceData{
	static constexpr int MAX_FACE_LEN = 6;
	std::set<VtkFace> faces;

	int n_faces() const {
		return (int)faces.size();
	}

	std::vector<std::vector<int>> faces_to_vec() const{
		std::vector<std::vector<int>> ret;
		for (auto& it: faces){
			ret.push_back(it.points_vec());
		}
		return ret;
	}

	void add_face(const std::vector<int>& face, int icell){
		bool was_reverted;
		auto ires = faces.insert(VtkFace(face, was_reverted));
		VtkFace& fc = const_cast<VtkFace&>(*ires.first);
		if (was_reverted){
			fc.right_cell = icell;
		} else {
			fc.left_cell = icell;
		}
	}

	const VtkFace* find_face(const std::vector<int>& points){
		bool r;
		VtkFace f(points, r);
		auto fnd = faces.find(f);
		if (fnd == faces.end()){
			return nullptr;
		} else {
			return &(*fnd);
		}
	}
};

VtkFaceData assemble_faces_vtk(const std::vector<std::shared_ptr<AVtkCell>>& cells){
	VtkFaceData ret;
	for (int icell=0; icell<(int)cells.size(); ++icell){
		for (const std::vector<int>& face: cells[icell]->faces()){
			ret.add_face(face, icell);
		}
	}
	int index = 0;
	for (auto& f: ret.faces){
		VtkFace& f2 = const_cast<VtkFace&>(f);
		f2.index = index++;
	}
	return ret;
}

struct ELineNotFound: public std::runtime_error{
	ELineNotFound(std::string s): std::runtime_error(s + " line not found while reading input") {};
};
std::string get_line_by_start(std::string start, std::istream& is){
	std::string line;
	while (is){
		std::getline(is, line);
		if (line.substr(0, start.size()) == start){
			return line;
		}
	}
	throw ELineNotFound(start);
}

}

std::shared_ptr<Grid> GridBuilder::build_from_gmshvtk(std::string fn){
	std::cout << "Reading grid from " << fn << std::endl;
	
	std::ifstream ifs(fn);
	if (!ifs){
		throw std::runtime_error(fn + " is not found");
	}
	std::string line, tmp;
	// header
	line = get_line_by_start("DATASET", ifs);
	if (line.substr(8) != "UNSTRUCTURED_GRID"){
		throw std::runtime_error("Only unstructured grid can be read");
	}
	// points
	line = get_line_by_start("POINTS", ifs);
	int n_points;
	std::istringstream(line) >> tmp >> n_points;

	std::vector<Point> points(n_points);
	for (int i=0; i<n_points; ++i){
		ifs >> points[i].x >> points[i].y >> tmp;
	}
	std::cout << n_points << " points" << std::endl;

	// cells
	int n_totals, n_cells;
	line = get_line_by_start("CELLS", ifs);
	std::istringstream(line) >> tmp >> n_cells >> n_totals;
	std::vector<int> totals(n_totals);
	std::vector<int> types(n_cells);
	std::vector<int> bnd(n_cells, -1);
	// -- totals
	for (int i=0; i<n_totals; ++i){
		ifs >> totals[i];
	}
	// -- types
	line = get_line_by_start("CELL_TYPES", ifs);
	for (int i=0; i<n_cells; ++i){
		ifs >> types[i];
	}
	// -- bnd if exists
	try {
		line = get_line_by_start("SCALARS CellEntityIds", ifs);
		line = get_line_by_start("LOOKUP_TABLE", ifs);
		for (int i=0; i<n_cells; ++i){
			ifs >> bnd[i];
		}
	} catch (ELineNotFound){
	}

	// assemble vtk cells
	std::vector<std::shared_ptr<AVtkCell>> vtk_cells;
	int* cursor = &totals[0];
	int geometry_dimension = 0;
	for (int i=0; i<n_cells; ++i){
		int len = cursor[0];
		cursor++;
		vtk_cells.push_back(AVtkCell::build(
			std::vector<int>(cursor, cursor + len),
			types[i]));
		geometry_dimension = std::max(geometry_dimension, vtk_cells.back()->dim());
		cursor += len;
	}

	// get internal and boundary vtk cells
	std::vector<std::shared_ptr<AVtkCell>> internal_cells;
	std::map<int, std::vector<std::shared_ptr<AVtkCell>>> boundary_cells;
	int n_boundary_cells = 0;
	for (int i=0; i<n_cells; ++i){
		int dim = vtk_cells[i]->dim();
		if (dim == geometry_dimension){
			internal_cells.push_back(vtk_cells[i]);
		} else if (dim == geometry_dimension-1 && bnd[i] > 0){
			boundary_cells[bnd[i]].push_back(vtk_cells[i]);
			n_boundary_cells += 1;
		}
	}
	std::cout << internal_cells.size() << " " << geometry_dimension << "D cells" << std::endl;
	std::cout << n_boundary_cells << " boundary faces with given btype" << std::endl;

	// build faces
	VtkFaceData face_data = assemble_faces_vtk(internal_cells);
	
	// build return grid
	std::shared_ptr<Grid> ret(new Grid(geometry_dimension));

	// fill points
	ret->_points = points;

	// fill cells
	for (const auto& c: internal_cells) {
		ret->_cells.push_back(c->points);
		ret->_vtk_cell_codes.push_back(c->code());
	}

	// fill faces and face->cell connectivity
	ret->_tab_face_cell.resize(face_data.n_faces());
	for (const VtkFace& f: face_data.faces){
		ret->_faces.push_back(f.points_vec());
		ret->_tab_face_cell[f.index].left_cell = f.left_cell;
		ret->_tab_face_cell[f.index].right_cell = f.right_cell;
	}

	// fill boundary
	for (auto& bit: boundary_cells){
		std::vector<int> bnd_faces;

		for (const std::shared_ptr<AVtkCell>& bp: bit.second){
			const VtkFace* fnd = face_data.find_face(bp->points);
			if (fnd == nullptr || !fnd->is_boundary()){
				throw std::runtime_error("Error in vtk face assembly");
			}
			bnd_faces.push_back(fnd->index);
		}

		ret->_boundaries.emplace(
			bit.first,
			GridBoundary(ret.get(), std::move(bnd_faces)));
	}

	return ret;
}

std::shared_ptr<Grid> GridBuilder::build_regular1(double len, int n_points){
	std::shared_ptr<Grid> ret(new Grid(1));

	for (int i=0; i<n_points; ++i){
		ret->_points.push_back({i*len/(n_points-1), 0, 0});
		ret->_faces.push_back(std::vector<int>{i});

		Grid::FaceCellEntry fc; 
		fc.left_cell = i-1;
		fc.right_cell = (i<n_points-1) ? i : -1;
		ret->_tab_face_cell.push_back(fc);
	}

	for (int i=0; i<n_points-1; ++i){
		ret->_cells.push_back(std::vector<int>{i, i+1});
		ret->_vtk_cell_codes.push_back((int)CellCode::SEGMENT);
	}

	ret->_boundaries.emplace(1, GridBoundary(ret.get(), {0}));
	ret->_boundaries.emplace(2, GridBoundary(ret.get(), {n_points-1}));

	return ret;
}
