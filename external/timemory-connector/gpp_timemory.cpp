

#include "gpp_timemory.hpp"

#include <ctime>
#include <execinfo.h>

// static int      current_region_level = 0;
static uint64_t uniqID = 0;

static string_t                         components_string;
static std::set<string_t>               components_string_set;
static std::vector<TIMEMORY_COMPONENT>  components_vector;
static std::map<std::string, uint64_t>  components_keys;
static std::map<uint64_t, auto_list_t*> parallel_map;
static string_t spacer = "---------------------------------------------------------";

//--------------------------------------------------------------------------------------//
//
//      TiMemory start/stop
//
//--------------------------------------------------------------------------------------//

void
parallel_start(const char* name, uint32_t devid, uint64_t* kernid)
{
    auto itr = components_keys.find(name);
    if(itr != components_keys.end())
    {
        *kernid = itr->second;
        // conditional prevents starting if already started, which can result in loss of
        // data
        parallel_map[*kernid]->conditional_start();
    }
    else
    {
        *kernid               = uniqID++;
        parallel_map[*kernid] = convert_to_object(name, *kernid, components_vector);
    }
}

void
parallel_stop(uint64_t kernid)
{
    // conditional prevents stopping if already stopped, which can result in duplicated
    // data
    parallel_map[kernid]->conditional_stop();
}

//--------------------------------------------------------------------------------------//
//
//      GPP symbols that get over-ridden with LD_PRELOAD
//
//--------------------------------------------------------------------------------------//

extern "C" void
gpp_init_library(const int loadSeq, const uint64_t interfaceVer,
                 const uint32_t devInfoCount, void* deviceInfo)
{
    printf("%s\n", spacer.c_str());
    printf("gpp-perf: TiMemory Connector (sequence is %d, version: %llu)\n", loadSeq,
           (long long int) interfaceVer);
    printf("%s\n\n", spacer.c_str());

    char* hostname = (char*) malloc(sizeof(char) * 256);
    gethostname(hostname, 256);

    std::stringstream folder;
    folder << hostname << "_" << current_date_time();
    free(hostname);

    tim::settings::auto_output()  = true;   // print when destructing
    tim::settings::text_output()  = true;   // print text files
    tim::settings::json_output()  = false;  // print to json
    tim::settings::timing_width() = 12;
    tim::settings::memory_width() = 12;
    tim::settings::memory_units() = "KB";

    std::stringstream ss;
    ss << loadSeq << "-" << interfaceVer << "-" << devInfoCount;
    auto cstr = const_cast<char*>(ss.str().c_str());

    tim::timemory_init(1, &cstr, "", "");

    tim::settings::cout_output() = false;  // print to stdout
    tim::settings::output_path() = folder.str();

    components_string =
        GetEnv<string_t>("KOKKOS_TIMEMORY_COMPONENTS", get_default_components());
    components_string_set = delimit(components_string);
    components_vector     = convert_string_set(components_string_set);
}

extern "C" void
gpp_finalize_library()
{
    printf("\n%s\n", spacer.c_str());
    printf("gpp-perf: Finalization of TiMemory Connector. Complete.\n");
    printf("%s\n\n", spacer.c_str());

    for(auto& itr : parallel_map)
    {
        std::cout << *itr.second << std::endl;
        delete itr.second;
    }
    std::cout << std::endl;
    parallel_map.clear();
}

extern "C" void
gpp_begin_record(const char* name, uint32_t devid, uint64_t* kernid)
{
    parallel_start(name, devid, kernid);
}

extern "C" void
gpp_end_record(uint64_t kernid)
{
    parallel_stop(kernid);
}
