#include <iostream>
#include "common.hpp"
#include "prog_common.hpp"
#include "prob/poisson_solver.hpp"
#include "grid/grid_builder.hpp"
#include "appr/linear_fem_approximator.hpp"
#include "appr/fvm_approximator.hpp"

void linear_fvm2(){
	// grid
	std::string grid_filename = from_input_path("rect1.vtk");
	std::shared_ptr<Grid> grid = GridBuilder::build_from_gmshvtk(grid_filename);

	// spatial approximator
	std::shared_ptr<FvmApproximator> appr = FvmApproximator::build(grid);

	// solver
	PoissonSolver slv(appr);

	// bc
	slv.set_bc_dirichlet(1, 0);
	slv.set_bc_dirichlet(4, 1);

	// rhs
	std::vector<double> rhs = appr->approximate([](Point p){ return 0; });

	// solve
	std::vector<double> x;
	slv.initialize();
	slv.solve(rhs, x);

	// show solution
	appr->vtk_save_scalar(from_output_path("poisson_fvm.vtk"), x);

	// check solution
	CHECK_FLOAT3(x[819], 0.1431);
}

void linear_fem2(){
	// grid
	std::string grid_filename = from_input_path("rect1.vtk");
	std::shared_ptr<Grid> grid = GridBuilder::build_from_gmshvtk(grid_filename);

	// spatial approximator
	std::shared_ptr<LinearFemApproximator> appr = LinearFemApproximator::build(grid);

	// solver
	PoissonSolver slv(appr);

	// bc
	slv.set_bc_dirichlet(1, 0);
	slv.set_bc_dirichlet(4, 1);

	// rhs
	std::vector<double> rhs = appr->approximate([](Point p){ return 0; });

	// solve
	std::vector<double> x;
	slv.initialize();
	slv.solve(rhs, x);

	// show solution
	appr->vtk_save_scalar(from_output_path("poisson_fem.vtk"), x);

	// check solution
	CHECK_FLOAT3(x[150], 0);
	CHECK_FLOAT3(x[446], 0.477941);
	CHECK_FLOAT3(x[2], 1);
}

int main(){
	try{
		linear_fvm2();
		linear_fem2();

		std::cout << "DONE" << std::endl;
	} catch (std::exception& e){
		std::cout << "ERROR: " << " " << e.what() << std::endl;
	}
}