#pragma once

#include <boost/test/unit_test.hpp>
#include "utils/Parallel.hpp"

#ifndef PRECICE_NO_PETSC
#include "petscsys.h"
#endif


namespace precice { namespace testing {

namespace bt = boost::unit_test;
using Par = precice::utils::Parallel;

/// Fixture to set and reset MPI communicator
struct MPICommRestrictFixture {
  explicit MPICommRestrictFixture(std::vector<int> & ranks)
  {
    // Restriction must always be called on all ranks, otherwise we hang
    if (static_cast<int>(ranks.size()) == Par::getCommunicatorSize()) {
      Par::setGlobalCommunicator(Par::getRestrictedCommunicator(ranks));
    }

    // if we set always set PETSC_COMM_WORLD to the restricted communicator, PETSc crashes
    #ifndef PRECICE_NO_PETSC
    if (ranks.size() == 1)
      PETSC_COMM_WORLD = PETSC_COMM_SELF;
    #endif
  }
  
  ~MPICommRestrictFixture()
  {
    Par::setGlobalCommunicator(Par::getCommunicatorWorld());
    #ifndef PRECICE_NO_PETSC
    PETSC_COMM_WORLD = MPI_COMM_WORLD;
    #endif
  }
};


/// Boost.Test decorator that makes the test run only on specfic ranks and restricts the MPI communicator
class OnRanks : public bt::decorator::base
{
public:

  explicit OnRanks(const std::vector<int> & ranks) :
    _ranks(ranks)
  {}

private:
  
  virtual void apply(bt::test_unit& tu)
  {
    size_t myRank = Par::getProcessRank();
    size_t size = Par::getCommunicatorSize();
    
    // If current rank is not in requested ranks
    if (std::find(_ranks.begin(), _ranks.end(), myRank) == _ranks.end()) {
      bt::framework::get<bt::test_suite>(tu.p_parent_id).remove(tu.p_id);
      return;
    }

    // If more ranks requested than available
    if (_ranks.size() > size) {
      bt::framework::get<bt::test_suite>(tu.p_parent_id).remove(tu.p_id);
      return;
    }

    // Install the fixture
    tu.p_fixtures.value.push_back(
      bt::test_unit_fixture_ptr(
        new bt::class_based_fixture<MPICommRestrictFixture, std::vector<int>>(_ranks)));

  }

  virtual bt::decorator::base_ptr clone() const
  {
    return bt::decorator::base_ptr(new OnRanks(_ranks));
  }

  std::vector<int> _ranks;
  
};

/// Boost.Test decorator that makes the test run only on the master aka rank 0
class OnMaster : public OnRanks
{
public:
  explicit OnMaster() :
    OnRanks({0})
  {}
};

}} // namespace precice, testing
