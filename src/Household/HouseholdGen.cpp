#include <cstdio>
#include <iostream>
#include <random>

#include "../../include/TBABM/HouseholdGen.h"

  shared_p<Household>
HouseholdGen::GetHousehold(const int current_time, const int hid, RNG& rng)
{
  // Create a blank Household object
  auto household = std::make_shared<Household>(current_time, hid);

  // Retrieve the corresponding vector
  size_t size = families.size();
  size_t idx = rng.mt_() % size;
  MicroFamily family = families[idx];

  // The first element in the vector is the head of the
  //   household; create an Individual using the smaller
  //   constructor
  MicroIndividual _head = family[0];

  auto initSimContext = CreateIndividualSimContext(current_time, 
      event_queue, 
      rng,
      fileData,
      params
      );

  auto initData = GetInitData();

  auto head = makeIndividual(
      initSimContext,
      initData,
      initHandles,
      name_gen.getName(rng), 
      hid, 
      0-365*_head.age, 
      _head.sex, 
      _head.role, 
      MarriageStatus::Single
      );

  // Add this object to the household as the head
  household->AddIndividual(head, current_time, HouseholdPosition::Head);

  // Go through the remaining elements:
  //   Establish relationships between the head
  //   and the current person under consideration
  //   Add them to the household
  // Return the household
  for (int i = 1; i < family.size(); ++i)
  {
    MicroIndividual midv = family[i];
    auto idv = makeIndividual(
        initSimContext,
        initData,
        initHandles,
        name_gen.getName(rng), 
        hid, 
        0-365*midv.age, 
        midv.sex, 
        midv.role, 
        MarriageStatus::Single
        );

    switch (idv->householdPosition) {
      case (HouseholdPosition::Head):
        std::cout << "Error: Can't have two household heads" << std::endl;
        break;

      case (HouseholdPosition::Spouse):
        idv->marriageStatus = MarriageStatus::Married;
        idv->spouse = household->head; // bidirectional
        household->head->spouse = idv;
        household->head->marriageStatus = MarriageStatus::Married;

        household->AddIndividual(idv, current_time, HouseholdPosition::Spouse);
        break;

      case (HouseholdPosition::Offspring):
        // Add offspring to head and spouse
        household->head->offspring.push_back(idv);
        if (household->spouse)
          household->spouse->offspring.push_back(idv);

        // Add offspring to household
        household->AddIndividual(idv, current_time, HouseholdPosition::Offspring);

        // Add mat/paternity to offspring
        if (household->head->sex == Sex::Male)
          idv->father = household->head;
        else
          idv->mother = household->head;

        if (household->spouse) {
          if (household->spouse->sex == Sex::Female) {
            idv->mother = household->spouse;
          } else {
            idv->father = household->spouse;
          }
        }
        break;

      case (HouseholdPosition::Other):
        idv->marriageStatus = MarriageStatus::Single;
        household->AddIndividual(idv, current_time, HouseholdPosition::Other);
        break;

      default:
        printf("Error: Unsupported HouseholdPosition\n");
        exit(1);
    }
  }

  return household;
}
