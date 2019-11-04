#include <unistd.h>
#include <vector>
#include <iostream>
#include <string>
#include <future>
#include <sys/stat.h>
#include <cstdint>

#include <Normal.h>
#include <RNG.h>
#include <JSONParameterize.h>
#include <JSONImport.h>
#include <docopt.h>

#include "../include/TBABM/TBABM.h"
#include "../include/TBABM/utils/threadpool.h"

using Constants = TBABM::Constants;

using namespace StatisticalDistributions;
using namespace SimulationLib::JSONImport;

using std::vector;
using std::string;
using std::future;

TimeSeriesExport<int> births;
TimeSeriesExport<int> deaths;
TimeSeriesExport<int> marriages;
TimeSeriesExport<int> divorces;
TimeSeriesExport<int> households;
TimeSeriesExport<int> singleToLooking;

TimeSeriesExport<int> populationSize;
TimeSeriesExport<int> populationChildren;
TimeSeriesExport<int> populationAdults;

TimeSeriesExport<int> hivNegative;
TimeSeriesExport<int> hivPositive;
TimeSeriesExport<int> hivPositiveART;

TimeSeriesExport<int> hivInfections;
TimeSeriesExport<int> hivDiagnosed;
TimeSeriesExport<int> hivDiagnosedVCT;
TimeSeriesExport<int> hivDiagnosesVCT;

TimeSeriesExport<int> tbInfections;
TimeSeriesExport<int> tbIncidence;
TimeSeriesExport<int> tbRecoveries;

TimeSeriesExport<int> tbInfectionsHousehold;
TimeSeriesExport<int> tbInfectionsCommunity;

TimeSeriesExport<int> tbSusceptible;
TimeSeriesExport<int> tbLatent;
TimeSeriesExport<int> tbInfectious;

TimeSeriesExport<int> tbExperienced;

TimeSeriesExport<int> tbTreatmentBegin;
TimeSeriesExport<int> tbTreatmentBeginHIV;
TimeSeriesExport<int> tbTreatmentBeginChildren;
TimeSeriesExport<int> tbTreatmentBeginAdultsNaive;
TimeSeriesExport<int> tbTreatmentBeginAdultsExperienced;
TimeSeriesExport<int> tbTreatmentEnd;
TimeSeriesExport<int> tbTreatmentDropout;

TimeSeriesExport<int> tbTxExperiencedAdults;
TimeSeriesExport<int> tbTxExperiencedInfectiousAdults;
TimeSeriesExport<int> tbTxNaiveAdults;
TimeSeriesExport<int> tbTxNaiveInfectiousAdults;

TimeSeriesExport<int> tbInTreatment;
TimeSeriesExport<int> tbCompletedTreatment;
TimeSeriesExport<int> tbDroppedTreatment;

PyramidTimeSeriesExport pyramid;
PyramidTimeSeriesExport deathPyramid;
PyramidTimeSeriesExport hivInfectionsPyramid;
PyramidTimeSeriesExport hivPositivePyramid;
PyramidTimeSeriesExport tbExperiencedPyramid;

map<TimeStatType, string> columns {
  {TimeStatType::Sum,  "Total"},
    {TimeStatType::Mean, "Average"},
    {TimeStatType::Min,  "Minimum"},
    {TimeStatType::Max,  "Maximum"}
};

TimeStatisticsExport activeHouseholdContacts(columns);

bool ExportTrajectory(TBABM& t, 
    std::uint_fast64_t i, 
    std::shared_ptr<ofstream> populationSurvey, 
    std::shared_ptr<ofstream> householdSurvey, 
    std::shared_ptr<ofstream> deathSurvey)
{
  bool success {true};
  long id {static_cast<long>(i)};

  auto data = t.GetData();

  success &= births.Add(                 std::move(std::make_shared<decltype(data.births)>(data.births)), id);
  success &= deaths.Add(                 std::move(std::make_shared<decltype(data.deaths)>(data.deaths)), id);
  success &= marriages.Add(              std::move(std::make_shared<decltype(data.marriages)>(data.marriages)), id);
  success &= divorces.Add(               std::move(std::make_shared<decltype(data.divorces)>(data.divorces)), id);
  success &= households.Add(             std::move(std::make_shared<decltype(data.householdsCount)>(data.householdsCount)), id);
  success &= singleToLooking.Add(        std::move(std::make_shared<decltype(data.singleToLooking)>(data.singleToLooking)), id);

  success &= populationSize.Add(         std::move(std::make_shared<decltype(data.populationSize)>(data.populationSize)), id);
  success &= populationChildren.Add(     std::move(std::make_shared<decltype(data.populationChildren)>(data.populationChildren)), id);
  success &= populationAdults.Add(       std::move(std::make_shared<decltype(data.populationAdults)>(data.populationAdults)), id);

  success &= hivNegative.Add(            std::move(std::make_shared<decltype(data.hivNegative)>(data.hivNegative)), id);
  success &= hivPositive.Add(            std::move(std::make_shared<decltype(data.hivPositive)>(data.hivPositive)), id);
  success &= hivPositiveART.Add(         std::move(std::make_shared<decltype(data.hivPositiveART)>(data.hivPositiveART)), id);
  success &= hivInfections.Add(          std::move(std::make_shared<decltype(data.hivInfections)>(data.hivInfections)), id);
  success &= hivDiagnosed.Add(           std::move(std::make_shared<decltype(data.hivDiagnosed)>(data.hivDiagnosed)), id);
  success &= hivDiagnosedVCT.Add(        std::move(std::make_shared<decltype(data.hivDiagnosedVCT)>(data.hivDiagnosedVCT)), id);
  success &= hivDiagnosesVCT.Add(        std::move(std::make_shared<decltype(data.hivDiagnosesVCT)>(data.hivDiagnosesVCT)), id);
  success &= tbInfections.Add(           std::move(std::make_shared<decltype(data.tbInfections)>(data.tbInfections)), id);
  success &= tbIncidence.Add(            std::move(std::make_shared<decltype(data.tbIncidence)>(data.tbIncidence)), id);
  success &= tbRecoveries.Add(           std::move(std::make_shared<decltype(data.tbRecoveries)>(data.tbRecoveries)), id);
  success &= tbInfectionsHousehold.Add(  std::move(std::make_shared<decltype(data.tbInfectionsHousehold)>(data.tbInfectionsHousehold)), id);
  success &= tbInfectionsCommunity.Add(  std::move(std::make_shared<decltype(data.tbInfectionsCommunity)>(data.tbInfectionsCommunity)), id);
  success &= tbSusceptible.Add(          std::move(std::make_shared<decltype(data.tbSusceptible)>(data.tbSusceptible)), id);
  success &= tbLatent.Add(               std::move(std::make_shared<decltype(data.tbLatent)>(data.tbLatent)), id);
  success &= tbInfectious.Add(           std::move(std::make_shared<decltype(data.tbInfectious)>(data.tbInfectious)), id);
  success &= tbExperienced.Add(		   std::move(std::make_shared<decltype(data.tbExperienced)>(data.tbExperienced)), id);
  success &= tbTreatmentBegin.Add(       std::move(std::make_shared<decltype(data.tbTreatmentBegin)>(data.tbTreatmentBegin)), id);
  success &= tbTreatmentBeginHIV.Add(    std::move(std::make_shared<decltype(data.tbTreatmentBeginHIV)>(data.tbTreatmentBeginHIV)), id);
  success &= tbTreatmentBeginChildren.Add(    std::move(std::make_shared<decltype(data.tbTreatmentBeginChildren)>(data.tbTreatmentBeginChildren)), id);
  success &= tbTreatmentBeginAdultsNaive.Add(    std::move(std::make_shared<decltype(data.tbTreatmentBeginAdultsNaive)>(data.tbTreatmentBeginAdultsNaive)), id);
  success &= tbTreatmentBeginAdultsExperienced.Add(    std::move(std::make_shared<decltype(data.tbTreatmentBeginAdultsExperienced)>(data.tbTreatmentBeginAdultsExperienced)), id);
  success &= tbTreatmentEnd.Add(         std::move(std::make_shared<decltype(data.tbTreatmentEnd)>(data.tbTreatmentEnd)), id);
  success &= tbTreatmentDropout.Add(     std::move(std::make_shared<decltype(data.tbTreatmentDropout)>(data.tbTreatmentDropout)), id);
  success &= tbInTreatment.Add(          std::move(std::make_shared<decltype(data.tbInTreatment)>(data.tbInTreatment)), id);
  success &= tbCompletedTreatment.Add(   std::move(std::make_shared<decltype(data.tbCompletedTreatment)>(data.tbCompletedTreatment)), id);
  success &= tbDroppedTreatment.Add(     std::move(std::make_shared<decltype(data.tbDroppedTreatment)>(data.tbDroppedTreatment)), id);
  success &= pyramid.Add(                std::move(std::make_shared<decltype(data.pyramid)>(data.pyramid)), id);
  success &= deathPyramid.Add(           std::move(std::make_shared<decltype(data.deathPyramid)>(data.deathPyramid)), id);
  success &= hivInfectionsPyramid.Add(   std::move(std::make_shared<decltype(data.hivInfectionsPyramid)>(data.hivInfectionsPyramid)), id);
  success &= hivPositivePyramid.Add(     std::move(std::make_shared<decltype(data.hivPositivePyramid)>(data.hivPositivePyramid)), id);
  success &= tbExperiencedPyramid.Add(   std::move(std::make_shared<decltype(data.tbExperiencedPyr)>(data.tbExperiencedPyr)), id);

  success &= tbTxExperiencedAdults.Add(               std::move(std::make_shared<decltype(data.tbTxExperiencedAdults)>(data.tbTxExperiencedAdults)), id);
  success &= tbTxExperiencedInfectiousAdults.Add(     std::move(std::make_shared<decltype(data.tbTxExperiencedInfectiousAdults)>(data.tbTxExperiencedInfectiousAdults)), id);
  success &= tbTxNaiveAdults.Add(                     std::move(std::make_shared<decltype(data.tbTxNaiveAdults)>(data.tbTxNaiveAdults)), id);
  success &= tbTxNaiveInfectiousAdults.Add(           std::move(std::make_shared<decltype(data.tbTxNaiveInfectiousAdults)>(data.tbTxNaiveInfectiousAdults)), id);


  success &= activeHouseholdContacts.Add(std::move(std::make_shared<decltype(data.activeHouseholdContacts)>(data.activeHouseholdContacts)));

  success &= t.WriteSurveys(populationSurvey, householdSurvey, deathSurvey);

  return success;
}

bool WriteData(string outputPrefix)
{
  return (
      births.Write(outputPrefix + "births.csv")               &&                                             
      deaths.Write(outputPrefix + "deaths.csv")               &&                                             
      marriages.Write(outputPrefix + "marriages.csv")            &&                                    
      divorces.Write(outputPrefix + "divorces.csv")             &&                                       
      singleToLooking.Write(outputPrefix + "singleToLooking.csv")      &&                  

      populationSize.Write(outputPrefix + "populationSize.csv")       &&                     
      populationChildren.Write(outputPrefix + "populationChildren.csv")       &&                     
      populationAdults.Write(outputPrefix + "populationAdults.csv")       &&                     

      pyramid.Write(outputPrefix + "pyramid.csv")              &&                                          
      deathPyramid.Write(outputPrefix + "deathPyramid.csv")         &&                           
      households.Write(outputPrefix + "households.csv")           &&                                 

      hivInfectionsPyramid.Write(outputPrefix + "hivInfectionsPyramid.csv") &&   
      hivPositivePyramid.Write(outputPrefix + "hivPositivePyramid.csv")   &&         
      hivNegative.Write(outputPrefix + "hivNegative.csv")          &&                              
      hivPositive.Write(outputPrefix + "hivPositive.csv")          &&                              
      hivPositiveART.Write(outputPrefix + "hivPositiveART.csv")       &&                     
      hivInfections.Write(outputPrefix + "hivInfections.csv")        &&                        
      hivDiagnosed.Write(outputPrefix + "hivDiagnosed.csv")         &&                           
      hivDiagnosedVCT.Write(outputPrefix + "hivDiagnosedVCT.csv")      &&                  
      hivDiagnosesVCT.Write(outputPrefix + "hivDiagnosesVCT.csv")      &&                  

      tbInfections.Write(outputPrefix + "tbInfections.csv")         &&                           
      tbIncidence.Write(outputPrefix + "tbIncidence.csv")        &&                        
      tbRecoveries.Write(outputPrefix + "tbRecoveries.csv")         &&                           

      tbInfectionsHousehold.Write(outputPrefix + "tbInfectionsHousehold.csv")&&
      tbInfectionsCommunity.Write(outputPrefix + "tbInfectionsCommunity.csv")&&

      tbSusceptible.Write(outputPrefix + "tbSusceptible.csv")        &&                        
      tbLatent.Write(outputPrefix + "tbLatent.csv")             &&                                       
      tbInfectious.Write(outputPrefix + "tbInfectious.csv")         &&                           

      tbExperienced.Write(outputPrefix + "tbExperienced.csv") &&

      tbTreatmentBegin.Write(outputPrefix + "tbTreatmentBegin.csv")     &&               
      tbTreatmentBeginHIV.Write(outputPrefix + "tbTreatmentBeginHIV.csv")  &&
      tbTreatmentBeginChildren.Write(outputPrefix + "tbTreatmentBeginChildren.csv")  &&
      tbTreatmentBeginAdultsNaive.Write(outputPrefix + "tbTreatmentBeginAdultsNaive.csv")  &&
      tbTreatmentBeginAdultsExperienced.Write(outputPrefix + "tbTreatmentBeginAdultsExperienced.csv")  &&       

      tbTreatmentEnd.Write(outputPrefix + "tbTreatmentEnd.csv")       &&                     
      tbTreatmentDropout.Write(outputPrefix + "tbTreatmentDropout.csv")   &&         
      tbInTreatment.Write(outputPrefix + "tbInTreatment.csv")        &&                        
      tbCompletedTreatment.Write(outputPrefix + "tbCompletedTreatment.csv") &&   
      tbDroppedTreatment.Write(outputPrefix + "tbDroppedTreatment.csv")   &&   

      tbExperiencedPyramid.Write(outputPrefix + "tbExperiencedPyramid.csv") &&

      tbTxExperiencedAdults.Write(outputPrefix + "tbTxExperiencedAdults.csv") &&
      tbTxExperiencedInfectiousAdults.Write(outputPrefix + "tbTxExperiencedInfectiousAdults.csv") &&
      tbTxNaiveAdults.Write(outputPrefix + "tbTxNaiveAdults.csv") &&
      tbTxNaiveInfectiousAdults.Write(outputPrefix + "tbTxNaiveInfectiousAdults.csv") &&

      activeHouseholdContacts.Write(outputPrefix + "activeHouseholdContacts.csv")
      );
}

bool householdsFileValid(const string& fname)
{
  struct stat buf;
  return stat(fname.c_str(), &buf) == 0;
}

static const char USAGE[] =
R"(TBABM
    
    Usage:
      TBABM -t NUM -n NUM [options]

    Options:
      -t NUM     Number of trajectories to run.
      -n NUM     Initial approximate size of population.
      -p PATH    Path to the parameter file [default: sampleParams.json]
      -y NUM     Years to simulate [default: 50]
      -s NUM     Seed of the master PRNG. Default is system time
      -o PATH    Dir for outputs. Include trailing slash. [default: .]
      -m NUM     Size of threadpool [default: 1]
      -h PATH    Location of households file. [default: household_structure.csv]
      --version  Print version
)";

int main(int argc, char **argv)
{
  std::map<std::string, docopt::value> args
    = docopt::docopt(USAGE,
                     { argv+1, argv+argc },
                     true,                  // show help if requested
                     "TBABM 0.6.7-alpha2"); // version string

  for (auto const& arg : args) {
    std::cout << arg.first << arg.second << std::endl;
  }

  exit(0);
  // Required rguments:
  // -t INTEGER
  // 		The number of trajectories to run.
  // -n INTEGER
  // 		The initial population size.
  // 		
  // Optional arguments:
  // -p SHEET_NAME
  // 		Optional. Path to parameter sheet (as json file). If not specified,
  // 		uses 'sampleParams.json'
  // -y INTEGER
  // 		Optional. Specifies the number of years for the model to run.
  // -s INTEGER
  // 		Optional. RNG seed. Default is std::time(NULL).
  // -o FOLDER_NAME
  // 		Optional. Folder to store outputs in. Include trailing 
  // 		forward-slash. Otherwise, outputs to working dir
  // -m INTEGER
  // 	    Run in parallel with a pool size of INTEGER.
  // -h FILE_NAME
  // 		Load the specified households file. Defaults to household_structure.csv

  string opthelp {\
    "Required arguments:\n\
      -t INTEGER\n\
      The number of trajectories to run.\n\
      -n INTEGER\n\
      The initial population size.\n\
      \n\
      Optional arguments:\n\
      -p SHEET_NAME\n\
      Optional. Path to parameter sheet (as json file). If not specified,\n\
      uses 'sampleParams.json'\n\
      -y INTEGER\n\
      Optional. Specifies the number of years for the model to run.\n\
      -s INTEGER\n\
      Optional. RNG seed. Default is std::time(NULL).\n\
      -o FOLDER_NAME\n\
      Optional. Folder to store outputs in. Include trailing \n\
      forward-slash. Otherwise, outputs to working dir\n\
      -m INTEGER\n\
      Run in parallel with a pool size of INTEGER.\n\
      -h FILE_NAME\n\
      Load the specified households file. Defaults to household_structure.csv"};

  if (argc == 1) {
    cout << opthelp << std::endl;
    return EXIT_FAILURE;
  }

  Constants constants {};

  // Initialize some constants that will be passed to each trajectory
  constants["tMax"]           = 365*50;
  constants["periodLength"]   = 365;
  constants["ageGroupWidth"]  = 3;
  constants["startYear"]      = 1990;
  constants["populationSize"] = 10000;

  // Initialize a few default values for parameters that mostly can be 
  // passed through the command line
  string householdsFile {"household_structure.csv"};
  int nTrajectories {1};
  auto timestamp = static_cast<std::uint_fast64_t>(std::time(NULL));

  string folder {""};
  string parameter_sheet {"sampleParams.json"};

  int pool_size {1};

  int opt;
  while ((opt = getopt(argc, argv, ":t:n:p:y:s:o:m:h:")) != -1)
  {
    switch (opt)
    {
      case 'h':
        householdsFile = std::string(optarg);
        break;
      case 't':
        nTrajectories = atoi(optarg);
        break;
      case 'n':
        constants["populationSize"] = atoi(optarg);
        break;
      case 'p':
        parameter_sheet = optarg;
        break;
      case 'y':
        constants["tMax"] = 365 * atoi(optarg);
        break;
      case 's':
        timestamp = atol(optarg);
        break;
      case 'm':
        pool_size = atoi(optarg);
        break;
      case 'o':
        folder = std::string(optarg);
        break;
      case '?':
        printf("Illegal option!\n");
        exit(1);
        break;
    }
  }

  // Initialize the master RNG, and write the seed to the file "seed_log.txt"
  RNG rng(timestamp);

  // Come up with the prefix for file output, and create the directory the files
  // will reside in, if it isn't already there
  mkdir(folder.c_str(), S_IRWXU);
  string outputPrefix {folder /*+ to_string(timestamp) + "_"*/};

  // Check to make sure that a households file can be opened
  if (!householdsFileValid(householdsFile)) {
    printf("Error: households file '%s' invalid, exiting\n", householdsFile.c_str());
    exit(EXIT_FAILURE);
  }

  const std::map<string, string> surveyHeaders {
    {"population", "trajectory,time,hash,age,sex,marital,household,householdHash,offspring,mom,dad,HIV,ART,CD4,TBStatus"},
      {"household", "trajectory,time,hash,size,head,spouse,directOffspring,otherOffspring,other"},
      {"death", "trajectory,time,hash,age,sex,cause,HIV,HIV_date,ART,ART_date,CD4,baseline_CD4"}
  };
  std::map<string, std::shared_ptr<ofstream>> surveyFiles;

  auto prepareSurvey = [outputPrefix](std::pair<string, string> name_and_header) 
    -> std::pair<string, std::shared_ptr<ofstream>> {

      // Attempt to initialize files to output surveys to
      string fname {outputPrefix + name_and_header.first + ".csv"};

      auto temp = std::make_shared<ofstream>(fname, ios_base::out);

      // Handle initialization failures
      if (temp->fail()) {
        printf("Attempt to open file '%s' failed\n", fname.c_str());
        exit(EXIT_FAILURE);
      }

      *temp << name_and_header.second << std::endl;

      if (temp->fail())
        printf("Attempt to write header to file '%s' failed\n", fname.c_str());

      return std::make_pair(name_and_header.first, temp);
    };

  std::transform(surveyHeaders.begin(), 
      surveyHeaders.end(),
      std::inserter(surveyFiles, surveyFiles.begin()),
      prepareSurvey);

  // Initialize the map of simulation parameters
  std::map<string, Param> params{};
  mapShortNames( fileToJSON(parameter_sheet), params );

  // Thread pool for trajectories, and associated futures
  ThreadPool pool(pool_size);
  vector<future<bool>> results;

  // Mutex lock for data-export critical section
  std::mutex mtx;

  std::vector<std::uint_fast64_t> seeds;
  for (int i = 0; i < nTrajectories; i++)
    seeds.emplace_back(rng.mt_());

  printf("Finished processing arguments and initializing the pool\n");

  for (int i = 0; i < nTrajectories; i++) {
    results.emplace_back(
        pool.enqueue([i, &params, constants, householdsFile, seeds, &mtx, &surveyFiles] {
          printf("#%4d RUNNING\n", i);

          // Initialize a trajectory
          auto seed = seeds[i];
          auto traj = TBABM(params, 
              constants, 
              householdsFile.c_str(), 
              seeds[i]);

          // Run the trajectory and check its' status
          if (!traj.Run()) {
          printf("Trajectory %4d: Run() failed\n", i);
          return false;
          }

          // Acquire a mutex lock for the data-exporting critical section
          mtx.lock();

          // Pass the TBABM object to a function that will export its data
          if (!ExportTrajectory(traj, 
                seed,
                surveyFiles.at("population"),
                surveyFiles.at("household"),
                surveyFiles.at("death"))) {

            printf("Trajectory #%4d: ExportTrajectrory(1) failed\n", i);
            mtx.unlock();
            return false;
          }

          // Release the mutex lock and return
          mtx.unlock();

          // printf("#%4d FINISHED\n", i);
          return true;
        })
    );
  }

  // Barrier: Wait for each thread to return successfully or unsuccessfully
  // for (auto && result : results)
  // std::cout << result.get() << ' ';
  for (int i = 0; i < nTrajectories; i++) {
    printf("#%4d JOINED: %d\n", i, (int)results[i].get());
  }

  std::cout << std::endl;

  if (!WriteData(outputPrefix)) {
    printf("WriteData() failed. Exiting\n");
    exit(1);
  }

  for (auto && file : surveyFiles) {
    file.second->flush();
    file.second->close();
  }

  return 0;
}
