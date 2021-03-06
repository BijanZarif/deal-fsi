
/*
 * Author
 *   David Blom, TU Delft. All rights reserved.
 */

#include "include/LinearElasticity.h"

#define BOOST_TEST_NO_MAIN
#define BOOST_TEST_MODULE SolidMechanicsTest
#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_CASE( polynomial_degree_test )
{
    using namespace dealii;
    using namespace dealiifsi;

    deallog.depth_console( 0 );

    double time_step = 2.5e-3;
    double theta = 1;
    unsigned int degree = 1;
    unsigned int n_global_refines = 1;
    double gravity = 2;
    double distributed_load = 0;
    double rho = 1000;
    double final_time = 0.05;
    double nu = 0.4;
    double E = 1.4e6;

    unsigned int nbComputations = 4;

    std::vector<unsigned int> n_dofs( nbComputations );
    std::vector<double> solution( nbComputations );

    for ( unsigned int i = 0; i < nbComputations; ++i )
    {
        n_global_refines = i + 2;
        LinearElasticity<2> linear_elasticity_solver( time_step, final_time, theta, degree, gravity, distributed_load, rho, E, nu, n_global_refines );
        linear_elasticity_solver.run();

        n_dofs[i] = linear_elasticity_solver.n_dofs();
        solution[i] = linear_elasticity_solver.point_value();

        BOOST_CHECK_CLOSE( solution[i], -0.001351993, 0.1 );
    }

    std::vector<double> error( nbComputations - 1 );

    for ( unsigned int i = 0; i < error.size(); ++i )
        error[i] = std::abs( solution[i] - solution[nbComputations - 1] ) / std::abs( solution[nbComputations - 1] );

    for ( unsigned int i = 0; i < error.size() - 1; ++i )
    {
        double rate = 2 * std::log10( error[i] / error[i + 1] );
        rate /= std::log10( n_dofs[i + 1] / n_dofs[i] );

        BOOST_CHECK_GE( rate, 1.9 );
    }
}

BOOST_AUTO_TEST_CASE( polynomial_degree_test_distributed_load )
{
    using namespace dealii;
    using namespace dealiifsi;

    deallog.depth_console( 0 );

    double time_step = 2.5e-3;
    double theta = 1;
    unsigned int degree = 1;
    unsigned int n_global_refines = 1;
    double gravity = 0;
    double distributed_load = 49.757;
    double rho = 1000;
    double final_time = 0.05;
    double nu = 0.4;
    double E = 1.4e6;

    unsigned int nbComputations = 4;

    std::vector<unsigned int> n_dofs( nbComputations );
    std::vector<double> solution( nbComputations );

    for ( unsigned int i = 0; i < nbComputations; ++i )
    {
        n_global_refines = i + 2;
        LinearElasticity<2> linear_elasticity_solver( time_step, final_time, theta, degree, gravity, distributed_load, rho, E, nu, n_global_refines );
        linear_elasticity_solver.run();

        n_dofs[i] = linear_elasticity_solver.n_dofs();
        solution[i] = linear_elasticity_solver.point_value();

        BOOST_CHECK_CLOSE( solution[i], -0.0041534231108655752, 0.1 );
    }

    std::vector<double> error( nbComputations - 1 );

    for ( unsigned int i = 0; i < error.size(); ++i )
        error[i] = std::abs( solution[i] - solution[nbComputations - 1] ) / std::abs( solution[nbComputations - 1] );

    for ( unsigned int i = 0; i < error.size() - 1; ++i )
    {
        double rate = 2 * std::log10( error[i] / error[i + 1] );
        rate /= std::log10( n_dofs[i + 1] / n_dofs[i] );

        BOOST_CHECK_GE( rate, 1.9 );
    }
}

BOOST_AUTO_TEST_CASE( crank_nicolson_distributed_load )
{
    using namespace dealii;
    using namespace dealiifsi;

    deallog.depth_console( 0 );

    double time_step = 2.5e-3;
    double theta = 0.5;
    unsigned int degree = 1;
    unsigned int n_global_refines = 2;
    double gravity = 0;
    double distributed_load = 49.757;
    double rho = 1000;
    double final_time = 0.05;
    double nu = 0.4;
    double E = 1.4e6;

    unsigned int nbComputations = 4;

    std::vector<unsigned int> nbTimeSteps( nbComputations );
    std::vector<double> solution_l2_norm( nbComputations );

    for ( unsigned int i = 0; i < nbComputations; ++i )
    {
        double dt = time_step / std::pow( 2, i );

        LinearElasticity<2> linear_elasticity_solver( dt, final_time, theta, degree, gravity, distributed_load, rho, E, nu, n_global_refines );
        linear_elasticity_solver.run();

        if ( i > 0 )
            assert( nbTimeSteps[i - 1] * 2 == linear_elasticity_solver.timestep_number );

        Vector<double> localized_solution( linear_elasticity_solver.solution_v );

        solution_l2_norm[i] = localized_solution.l2_norm();
        nbTimeSteps[i] = linear_elasticity_solver.timestep_number;
    }

    std::vector<double> error( nbComputations - 1 );

    for ( unsigned int i = 0; i < solution_l2_norm.size(); ++i )
        std::cout << "l2norm = " << solution_l2_norm[i] << std::endl;

    for ( unsigned int i = 0; i < error.size(); ++i )
    {
        error[i] = std::abs( solution_l2_norm[i] - solution_l2_norm[nbComputations - 1] ) / std::abs( solution_l2_norm[nbComputations - 1] );

        std::cout << "error = " << error[i] << std::endl;
    }

    std::vector<double> order( nbComputations - 2 );

    for ( unsigned int i = 0; i < order.size(); ++i )
    {
        double dti = time_step / std::pow( 2, i );
        double dtinew = time_step / std::pow( 2, i + 1 );
        order[i] = std::log10( error[i + 1] ) - std::log10( error[i] );
        order[i] /= std::log10( dtinew ) - std::log10( dti );
        std::cout << "order = " << order[i] << std::endl;

        BOOST_CHECK_GE( order[i], 1.77 );
    }
}

BOOST_AUTO_TEST_CASE( crank_nicolson_combined_load )
{
    using namespace dealii;
    using namespace dealiifsi;

    deallog.depth_console( 0 );

    double time_step = 2.5e-3;
    double theta = 0.5;
    unsigned int degree = 1;
    unsigned int n_global_refines = 2;
    double gravity = 2;
    double distributed_load = 49.757;
    double rho = 1000;
    double final_time = 0.05;
    double nu = 0.4;
    double E = 1.4e6;

    unsigned int nbComputations = 4;

    std::vector<unsigned int> nbTimeSteps( nbComputations );
    std::vector<double> solution_l2_norm( nbComputations );

    for ( unsigned int i = 0; i < nbComputations; ++i )
    {
        double dt = time_step / std::pow( 2, i );

        LinearElasticity<2> linear_elasticity_solver( dt, final_time, theta, degree, gravity, distributed_load, rho, E, nu, n_global_refines );
        linear_elasticity_solver.run();

        if ( i > 0 )
            assert( nbTimeSteps[i - 1] * 2 == linear_elasticity_solver.timestep_number );

        Vector<double> localized_solution( linear_elasticity_solver.solution_v );

        solution_l2_norm[i] = localized_solution.l2_norm();
        nbTimeSteps[i] = linear_elasticity_solver.timestep_number;
    }

    std::vector<double> error( nbComputations - 1 );

    for ( unsigned int i = 0; i < solution_l2_norm.size(); ++i )
        std::cout << "l2norm = " << solution_l2_norm[i] << std::endl;

    for ( unsigned int i = 0; i < error.size(); ++i )
    {
        error[i] = std::abs( solution_l2_norm[i] - solution_l2_norm[nbComputations - 1] ) / std::abs( solution_l2_norm[nbComputations - 1] );

        std::cout << "error = " << error[i] << std::endl;
    }

    std::vector<double> order( nbComputations - 2 );

    for ( unsigned int i = 0; i < order.size(); ++i )
    {
        double dti = time_step / std::pow( 2, i );
        double dtinew = time_step / std::pow( 2, i + 1 );
        order[i] = std::log10( error[i + 1] ) - std::log10( error[i] );
        order[i] /= std::log10( dtinew ) - std::log10( dti );
        std::cout << "order = " << order[i] << std::endl;

        BOOST_CHECK_GE( order[i], 1.8 );
    }
}

BOOST_AUTO_TEST_CASE( crank_nicolson_test )
{
    using namespace dealii;
    using namespace dealiifsi;

    deallog.depth_console( 0 );

    double time_step = 2.5e-3;
    double theta = 0.5;
    unsigned int degree = 1;
    unsigned int n_global_refines = 2;
    double gravity = 2;
    double distributed_load = 0;
    double rho = 1000;
    double final_time = 0.05;
    double nu = 0.4;
    double E = 1.4e6;

    unsigned int nbComputations = 4;

    std::vector<unsigned int> nbTimeSteps( nbComputations );
    std::vector<double> solution_l2_norm( nbComputations );

    for ( unsigned int i = 0; i < nbComputations; ++i )
    {
        double dt = time_step / std::pow( 2, i );

        LinearElasticity<2> linear_elasticity_solver( dt, final_time, theta, degree, gravity, distributed_load, rho, E, nu, n_global_refines );
        linear_elasticity_solver.run();

        if ( i > 0 )
            assert( nbTimeSteps[i - 1] * 2 == linear_elasticity_solver.timestep_number );

        Vector<double> localized_solution( linear_elasticity_solver.solution_v );

        solution_l2_norm[i] = localized_solution.l2_norm();
        nbTimeSteps[i] = linear_elasticity_solver.timestep_number;
    }

    std::vector<double> error( nbComputations - 1 );

    for ( unsigned int i = 0; i < solution_l2_norm.size(); ++i )
        std::cout << "l2norm = " << solution_l2_norm[i] << std::endl;

    for ( unsigned int i = 0; i < error.size(); ++i )
    {
        error[i] = std::abs( solution_l2_norm[i] - solution_l2_norm[nbComputations - 1] ) / std::abs( solution_l2_norm[nbComputations - 1] );

        std::cout << "error = " << error[i] << std::endl;
    }

    std::vector<double> order( nbComputations - 2 );

    for ( unsigned int i = 0; i < order.size(); ++i )
    {
        double dti = time_step / std::pow( 2, i );
        double dtinew = time_step / std::pow( 2, i + 1 );
        order[i] = std::log10( error[i + 1] ) - std::log10( error[i] );
        order[i] /= std::log10( dtinew ) - std::log10( dti );
        std::cout << "order = " << order[i] << std::endl;

        BOOST_CHECK_GE( order[i], 1.85 );
    }
}

BOOST_AUTO_TEST_CASE( backward_euler )
{
    using namespace dealii;
    using namespace dealiifsi;

    deallog.depth_console( 0 );

    double time_step = 2.5e-3;
    double theta = 1;
    unsigned int degree = 1;
    unsigned int n_global_refines = 2;
    double gravity = 2;
    double distributed_load = 0;
    double rho = 1000;
    double final_time = 0.05;
    double nu = 0.4;
    double E = 1.4e6;

    unsigned int nbComputations = 4;

    std::vector<unsigned int> nbTimeSteps( nbComputations );
    std::vector<double> solution_l2_norm( nbComputations );

    for ( unsigned int i = 0; i < nbComputations; ++i )
    {
        double dt = time_step / std::pow( 2, i );

        LinearElasticity<2> linear_elasticity_solver( dt, final_time, theta, degree, gravity, distributed_load, rho, E, nu, n_global_refines );
        linear_elasticity_solver.run();

        if ( i > 0 )
            assert( nbTimeSteps[i - 1] * 2 == linear_elasticity_solver.timestep_number );

        Vector<double> localized_solution( linear_elasticity_solver.solution_v );

        solution_l2_norm[i] = localized_solution.l2_norm();
        nbTimeSteps[i] = linear_elasticity_solver.timestep_number;
    }

    std::vector<double> error( nbComputations - 1 );

    for ( unsigned int i = 0; i < error.size(); ++i )
    {
        error[i] = std::abs( solution_l2_norm[i] - solution_l2_norm[nbComputations - 1] ) / std::abs( solution_l2_norm[nbComputations - 1] );
        std::cout << "error = " << error[i] << std::endl;
    }

    std::vector<double> order( nbComputations - 2 );

    for ( unsigned int i = 0; i < order.size(); ++i )
    {
        double dti = time_step / std::pow( 2, i );
        double dtinew = time_step / std::pow( 2, i + 1 );
        order[i] = std::log10( error[i + 1] ) - std::log10( error[i] );
        order[i] /= std::log10( dtinew ) - std::log10( dti );

        std::cout << "order = " << order[i] << std::endl;

        BOOST_CHECK_GE( order[i], 1 );
    }
}

BOOST_AUTO_TEST_CASE( theta )
{
    using namespace dealii;
    using namespace dealiifsi;

    deallog.depth_console( 0 );

    double time_step = 2.5e-3;
    double theta = 0.6;
    unsigned int degree = 1;
    unsigned int n_global_refines = 2;
    double gravity = 2;
    double distributed_load = 0;
    double rho = 1000;
    double final_time = 0.05;
    double nu = 0.4;
    double E = 1.4e6;

    unsigned int nbComputations = 4;

    std::vector<unsigned int> nbTimeSteps( nbComputations );
    std::vector<double> solution_l2_norm( nbComputations );

    for ( unsigned int i = 0; i < nbComputations; ++i )
    {
        double dt = time_step / std::pow( 2, i );

        LinearElasticity<2> linear_elasticity_solver( dt, final_time, theta, degree, gravity, distributed_load, rho, E, nu, n_global_refines );
        linear_elasticity_solver.run();

        if ( i > 0 )
            assert( nbTimeSteps[i - 1] * 2 == linear_elasticity_solver.timestep_number );

        Vector<double> localized_solution( linear_elasticity_solver.solution_v );

        solution_l2_norm[i] = localized_solution.l2_norm();
        nbTimeSteps[i] = linear_elasticity_solver.timestep_number;
    }

    std::vector<double> error( nbComputations - 1 );

    for ( unsigned int i = 0; i < error.size(); ++i )
        error[i] = std::abs( solution_l2_norm[i] - solution_l2_norm[nbComputations - 1] ) / std::abs( solution_l2_norm[nbComputations - 1] );

    std::vector<double> order( nbComputations - 2 );

    for ( unsigned int i = 0; i < order.size(); ++i )
    {
        double dti = time_step / std::pow( 2, i );
        double dtinew = time_step / std::pow( 2, i + 1 );
        order[i] = std::log10( error[i + 1] ) - std::log10( error[i] );
        order[i] /= std::log10( dtinew ) - std::log10( dti );

        BOOST_CHECK_GE( order[i], 1 );
    }
}

BOOST_AUTO_TEST_CASE( writePositions )
{
    using namespace dealii;
    using namespace dealiifsi;

    double time_step = 2.5e-3;
    double theta = 0.6;
    unsigned int degree = 1;
    unsigned int n_global_refines = 0;
    double gravity = 2;
    double distributed_load = 0;
    double rho = 1000;
    double final_time = 0.05;
    double nu = 0.4;
    double E = 1.4e6;

    LinearElasticity<2> linear_elasticity_solver( time_step, final_time, theta, degree, gravity, distributed_load, rho, E, nu, n_global_refines );

    EigenMatrix writePositions;
    linear_elasticity_solver.getWritePositions( writePositions );

    MPI_Comm mpi_communicator( MPI_COMM_WORLD );
    const unsigned int n_mpi_processes = Utilities::MPI::n_mpi_processes( mpi_communicator );

    BOOST_CHECK_EQUAL( writePositions.cols(), 2 );
    BOOST_CHECK_GE( writePositions.rows(), 0 );

    if ( n_mpi_processes == 1 )
    {
        BOOST_CHECK_CLOSE( writePositions( 0, 0 ), 0.251109, 0.1 );
        BOOST_CHECK_CLOSE( writePositions( 0, 1 ), 0.19, 0.1 );
        BOOST_CHECK_CLOSE( writePositions( 1, 0 ), 0.2569, 0.1 );
        BOOST_CHECK_CLOSE( writePositions( 1, 1 ), 0.19, 0.1 );
    }
}

BOOST_AUTO_TEST_CASE( readPositions )
{
    using namespace dealii;
    using namespace dealiifsi;

    double time_step = 2.5e-3;
    double theta = 0.6;
    unsigned int degree = 1;
    unsigned int n_global_refines = 0;
    double gravity = 2;
    double distributed_load = 0;
    double rho = 1000;
    double final_time = 0.05;
    double nu = 0.4;
    double E = 1.4e6;

    LinearElasticity<2> linear_elasticity_solver( time_step, final_time, theta, degree, gravity, distributed_load, rho, E, nu, n_global_refines );

    EigenMatrix readPositions;
    linear_elasticity_solver.getReadPositions( readPositions );

    MPI_Comm mpi_communicator( MPI_COMM_WORLD );
    const unsigned int n_mpi_processes = Utilities::MPI::n_mpi_processes( mpi_communicator );

    BOOST_CHECK_EQUAL( readPositions.cols(), 2 );
    BOOST_CHECK_GE( readPositions.rows(), 0 );

    if ( n_mpi_processes == 1 )
    {
        BOOST_CHECK_CLOSE( readPositions( 0, 0 ), 0.251109, 0.1 );
        BOOST_CHECK_CLOSE( readPositions( 0, 1 ), 0.19, 0.1 );
        BOOST_CHECK_CLOSE( readPositions( 1, 0 ), 0.2569, 0.1 );
        BOOST_CHECK_CLOSE( readPositions( 1, 1 ), 0.19, 0.1 );
    }
}

BOOST_AUTO_TEST_CASE( displacement )
{
    using namespace dealii;
    using namespace dealiifsi;

    double time_step = 2.5e-3;
    double theta = 0.6;
    unsigned int degree = 1;
    unsigned int n_global_refines = 0;
    double gravity = 2;
    double distributed_load = 0;
    double rho = 1000;
    double final_time = 0.05;
    double nu = 0.4;
    double E = 1.4e6;

    LinearElasticity<2> linear_elasticity_solver( time_step, final_time, theta, degree, gravity, distributed_load, rho, E, nu, n_global_refines );

    EigenMatrix readPositions, displacement;
    linear_elasticity_solver.getReadPositions( readPositions );
    linear_elasticity_solver.getDisplacement( displacement );

    BOOST_CHECK_EQUAL( readPositions.cols(), displacement.cols() );
    BOOST_CHECK_EQUAL( readPositions.rows(), displacement.rows() );
    BOOST_CHECK_EQUAL( readPositions.cols(), 2 );
    BOOST_CHECK_GE( readPositions.rows(), 0 );
}

BOOST_AUTO_TEST_CASE( displacement_end )
{
    using namespace dealii;
    using namespace dealiifsi;

    double time_step = 2.5e-3;
    double theta = 0.6;
    unsigned int degree = 1;
    unsigned int n_global_refines = 0;
    double gravity = 2;
    double distributed_load = 0;
    double rho = 1000;
    double final_time = 0.05;
    double nu = 0.4;
    double E = 1.4e6;

    LinearElasticity<2> linear_elasticity_solver( time_step, final_time, theta, degree, gravity, distributed_load, rho, E, nu, n_global_refines );

    EigenMatrix readPositions, displacement;
    linear_elasticity_solver.getReadPositions( readPositions );
    linear_elasticity_solver.run();
    linear_elasticity_solver.getDisplacement( displacement );

    MPI_Comm mpi_communicator( MPI_COMM_WORLD );
    const unsigned int n_mpi_processes = Utilities::MPI::n_mpi_processes( mpi_communicator );

    BOOST_CHECK_EQUAL( readPositions.cols(), displacement.cols() );
    BOOST_CHECK_EQUAL( readPositions.rows(), displacement.rows() );
    BOOST_CHECK_EQUAL( readPositions.cols(), 2 );
    BOOST_CHECK_GE( readPositions.rows(), 0 );
    BOOST_CHECK_EQUAL( std::isnan( displacement.norm() ), false );
    BOOST_CHECK_EQUAL( std::isnan( readPositions.norm() ), false );

    if ( n_mpi_processes == 1 )
    {
        BOOST_CHECK_CLOSE( displacement( 0, 0 ), 0, 0.001 );
        BOOST_CHECK_CLOSE( displacement( 0, 1 ), 0, 0.001 );
        BOOST_CHECK_CLOSE( displacement( 1, 0 ), -2.08901000e-05, 0.1 );
        BOOST_CHECK_CLOSE( displacement( 1, 1 ), -2.08471000e-05, 0.1 );
        BOOST_CHECK_CLOSE( displacement( 2, 0 ), -4.18553000e-05, 0.1 );
        BOOST_CHECK_CLOSE( displacement( 2, 1 ), -5.65638000e-05, 0.1 );
    }
}

BOOST_AUTO_TEST_CASE( iterations )
{
    using namespace dealii;
    using namespace dealiifsi;

    double time_step = 2.5e-3;
    double theta = 0.6;
    unsigned int degree = 1;
    unsigned int n_global_refines = 0;
    double gravity = 2;
    double distributed_load = 0;
    double rho = 1000;
    double final_time = 0.05;
    double nu = 0.4;
    double E = 1.4e6;

    LinearElasticity<2> linear_elasticity_solver( time_step, final_time, theta, degree, gravity, distributed_load, rho, E, nu, n_global_refines );
    linear_elasticity_solver.run();

    EigenMatrix readPositions;
    linear_elasticity_solver.getReadPositions( readPositions );

    EigenMatrix displacement, displacement_2;
    linear_elasticity_solver.getDisplacement( displacement );

    LinearElasticity<2> linear_elasticity_solver_2( time_step, final_time, theta, degree, gravity, distributed_load, rho, E, nu, n_global_refines );

    while ( linear_elasticity_solver_2.isRunning() )
    {
        linear_elasticity_solver_2.initTimeStep();

        // Random number of iterations between 1 and 10
        unsigned int nbIter = rand() % 10 + 1;

        for ( unsigned int i = 0; i < nbIter; ++i )
            linear_elasticity_solver_2.solve();

        linear_elasticity_solver_2.finalizeTimeStep();
    }

    linear_elasticity_solver_2.getReadPositions( readPositions );
    linear_elasticity_solver_2.getDisplacement( displacement_2 );

    BOOST_CHECK_EQUAL( displacement.cols(), displacement_2.cols() );
    BOOST_CHECK_EQUAL( displacement.rows(), displacement_2.rows() );

    for ( unsigned int i = 0; i < displacement.rows(); ++i )
        for ( unsigned int j = 0; j < displacement.cols(); ++j )
            BOOST_CHECK_CLOSE( displacement( i, j ), displacement_2( i, j ), 0.0001 );
}

BOOST_AUTO_TEST_CASE( configuration_file )
{
    using namespace dealii;
    using namespace dealiifsi;

    DataStorage data;
    data.read_data( "deal-fsi.prm" );
    LinearElasticity<2> linear_elasticity_solver( data );
    linear_elasticity_solver.run();
}

int main(
    int argc,
    char * argv[]
    )
{
    try
    {
        dealii::Utilities::MPI::MPI_InitFinalize mpi_initialization( argc, argv, 1 );

        boost::unit_test::test_suite * init_unit_test_suite(
            int argc,
            char * argv[]
            );

        boost::unit_test::init_unit_test_func init_func = &init_unit_test_suite;

        return ::boost::unit_test::unit_test_main( init_func, argc, argv );
    }
    catch ( std::exception & exc )
    {
        std::cerr << std::endl << std::endl
                  << "----------------------------------------------------"
                  << std::endl;
        std::cerr << "Exception on processing: " << std::endl
                  << exc.what() << std::endl
                  << "Aborting!" << std::endl
                  << "----------------------------------------------------"
                  << std::endl;
        return 1;
    }
    catch ( ... )
    {
        std::cerr << std::endl << std::endl
                  << "----------------------------------------------------"
                  << std::endl;
        std::cerr << "Unknown exception!" << std::endl
                  << "Aborting!" << std::endl
                  << "----------------------------------------------------"
                  << std::endl;
        return 1;
    }
    return 0;
}
