
//
//
// This builds a Kokkos tool using TiMemory (timing + memory + hw counters)
//
// https://github.com/jrmadsen/TiMemory
//
// Author: Jonathan R. Madsen
//

#pragma once

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <limits>
#include <map>
#include <set>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <vector>

#include <timemory/timemory.hpp>

using string_t     = std::string;
using string_set_t = std::set<string_t>;

//--------------------------------------------------------------------------------------//
//  declare the allowed component types
//  components are in tim::component:: namespace but thats a lot to type...
//
using namespace tim::component;
using auto_list_t =
    tim::auto_list<real_clock, system_clock, user_clock, cpu_clock, monotonic_clock,
                   monotonic_raw_clock, thread_cpu_clock, process_cpu_clock, cpu_util,
                   thread_cpu_util, process_cpu_util, current_rss, peak_rss, stack_rss,
                   data_rss, num_swap, num_io_in, num_io_out, num_minor_page_faults,
                   num_major_page_faults, num_msg_sent, num_msg_recv, num_signals,
                   voluntary_context_switch, priority_context_switch, cuda_event>;

//--------------------------------------------------------------------------------------//
// use this type derived from the auto type because the "auto" types do template
// filtering if a component is marked as unavailable, e.g.:
//
//      namespace tim
//      {
//      namespace component
//      {
//
//      template <>
//      struct impl_available<data_rss> : std::false_type;
//
//      } // namespace component
//      } // namespace tim
//
using comp_list_t = typename auto_list_t::component_type;

//--------------------------------------------------------------------------------------//
// use for recording the time of the execution
//
inline const std::string
current_date_time()
{
    time_t    now = time(0);
    struct tm tstruct;
    char      buf[80];
    tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(buf, sizeof(buf), "at_%X", &tstruct);
    std::string sbuf(buf);
    sbuf.substr(0, sbuf.find_last_of(":"));
    while(sbuf.find(":") != std::string::npos)
    {
        sbuf.replace(sbuf.find(":"), 1, "_");
    }
    return sbuf;
}

//--------------------------------------------------------------------------------------//
// default components to record -- maybe should be empty?
//
inline string_t
get_default_components()
{
    return "real_clock, user_clock, system_clock, cpu_util, current_rss, peak_rss, "
           "cuda_event";
}

//--------------------------------------------------------------------------------------//
//  use this function to get an environment variable setting + a default if set, e.g.
//      int num_threads = GetEnv<int>("NUM_THREADS", 4);
//
template <typename _Tp>
_Tp
GetEnv(const string_t& env_id, _Tp _default = _Tp())
{
    char* env_var = std::getenv(env_id.c_str());
    if(env_var)
    {
        try
        {
            string_t           str_var = string_t(env_var);
            std::istringstream iss(str_var);
            _Tp                var = _Tp();
            iss >> var;
            return var;
        }
        catch(const std::exception& e)
        {
            std::stringstream ss;
            ss << e.what();
            fprintf(stderr, "%s\n", ss.str().c_str());
            return _default;
        }
    }
    // return default if not specified in environment
    return _default;
}

//--------------------------------------------------------------------------------------//
//  process the environment variable KOKKOS_TIMEMORY_COMPONENTS
//  (or something other string?) for the list of components to record
//
template <typename _Predicate = std::function<string_t(string_t)>>
inline string_set_t
delimit(const string_t& line, const string_set_t& delimiters = { ",; " },
        _Predicate&& predicate = [](string_t s) -> string_t { return s; })
{
    auto _get_first_not_of = [&delimiters](const string_t& _string, const size_t& _beg) {
        size_t _min = string_t::npos;
        for(const auto& itr : delimiters)
        {
            auto _tmp = _string.find_first_not_of(itr, _beg);
            _min      = std::min(_min, _tmp);
        }
        return _min;
    };

    auto _get_first_of = [&delimiters](const string_t& _string, const size_t& _beg) {
        size_t _min = string_t::npos;
        for(const auto& itr : delimiters)
        {
            auto _tmp = _string.find_first_of(itr, _beg);
            _min      = std::min(_min, _tmp);
        }
        return _min;
    };

    string_set_t _result;
    size_t       _beginp = 0;  // position that is the beginning of the new string
    size_t       _delimp = 0;  // position of the delimiter in the string
    while(_beginp < line.length() && _delimp < line.length())
    {
        // find the first character (starting at _end) that is not a delimiter
        _beginp = _get_first_not_of(line, _delimp);
        // if no a character after or at _end that is not a delimiter is not found
        // then we are done
        if(_beginp == string_t::npos)
        {
            break;
        }
        // starting at the position of the new string, find the next delimiter
        _delimp = _get_first_of(line, _beginp);
        // if(d2 == string_t::npos) { d2 = string_t::npos; }
        string_t _tmp = "";
        try
        {
            // starting at the position of the new string, get the characters
            // between this position and the next delimiter
            _tmp = line.substr(_beginp, _delimp - _beginp);
        }
        catch(std::exception& e)
        {
            // print the exception but don't fail, unless maybe it should?
            std::stringstream ss;
            ss << e.what();
            fprintf(stderr, "%s\n", ss.str().c_str());
        }
        // don't add empty strings
        if(!_tmp.empty())
        {
            _result.insert(predicate(_tmp));
        }
    }
    return _result;
}

//--------------------------------------------------------------------------------------//

inline std::vector<TIMEMORY_COMPONENT>
convert_string_set(string_set_t comp_set)
{
    std::vector<TIMEMORY_COMPONENT> comp_vec;
    // auto to_lower = [](string_t& _str) { for(auto& itr : _str) { itr = tolower(itr);
    // }};

    for(auto& itr : comp_set)
    {
        // to_lower(itr);
        if(itr == "real_clock")
        {
            comp_vec.push_back(WALL_CLOCK);
        }
        else if(itr == "system_clock")
        {
            comp_vec.push_back(SYS_CLOCK);
        }
        else if(itr == "user_clock")
        {
            comp_vec.push_back(USER_CLOCK);
        }
        else if(itr == "cpu_clock")
        {
            comp_vec.push_back(CPU_CLOCK);
        }
        else if(itr == "monotonic_clock")
        {
            comp_vec.push_back(MONOTONIC_CLOCK);
        }
        else if(itr == "monotonic_raw_clock")
        {
            comp_vec.push_back(MONOTONIC_RAW_CLOCK);
        }
        else if(itr == "thread_cpu_clock")
        {
            comp_vec.push_back(THREAD_CPU_CLOCK);
        }
        else if(itr == "process_cpu_clock")
        {
            comp_vec.push_back(PROCESS_CPU_CLOCK);
        }
        else if(itr == "cpu_util")
        {
            comp_vec.push_back(CPU_UTIL);
        }
        else if(itr == "process_cpu_util")
        {
            comp_vec.push_back(PROCESS_CPU_UTIL);
        }
        else if(itr == "thread_cpu_util")
        {
            comp_vec.push_back(THREAD_CPU_UTIL);
        }
        else if(itr == "peak_rss")
        {
            comp_vec.push_back(PEAK_RSS);
        }
        else if(itr == "current_rss")
        {
            comp_vec.push_back(CURRENT_RSS);
        }
        else if(itr == "stack_rss")
        {
            comp_vec.push_back(STACK_RSS);
        }
        else if(itr == "data_rss")
        {
            comp_vec.push_back(DATA_RSS);
        }
        else if(itr == "num_swap")
        {
            comp_vec.push_back(NUM_SWAP);
        }
        else if(itr == "num_io_in")
        {
            comp_vec.push_back(NUM_IO_IN);
        }
        else if(itr == "num_io_out")
        {
            comp_vec.push_back(NUM_IO_OUT);
        }
        else if(itr == "num_minor_page_faults")
        {
            comp_vec.push_back(NUM_MINOR_PAGE_FAULTS);
        }
        else if(itr == "num_major_page_faults")
        {
            comp_vec.push_back(NUM_MAJOR_PAGE_FAULTS);
        }
        else if(itr == "num_msg_sent")
        {
            comp_vec.push_back(NUM_MSG_SENT);
        }
        else if(itr == "num_msg_recv")
        {
            comp_vec.push_back(NUM_MSG_RECV);
        }
        else if(itr == "num_signals")
        {
            comp_vec.push_back(NUM_SIGNALS);
        }
        else if(itr == "voluntary_context_switch")
        {
            comp_vec.push_back(VOLUNTARY_CONTEXT_SWITCH);
        }
        else if(itr == "priority_context_switch")
        {
            comp_vec.push_back(PRIORITY_CONTEXT_SWITCH);
        }
        else if(itr == "cuda_event")
        {
            comp_vec.push_back(CUDA_EVENT);
        }
        else
        {
            fprintf(stderr, "Unknown component label: %s\n", itr.c_str());
        }
    }
    return comp_vec;
}

//--------------------------------------------------------------------------------------//

auto_list_t*
convert_to_object(const string_t& tag, int kId,
                  const std::vector<TIMEMORY_COMPONENT>& components, bool report = false)
{
    using namespace tim::component;
    using data_type = typename auto_list_t::component_type::data_type;
    auto obj        = new auto_list_t(tag, kId, tim::language::cxx(), false);
    obj->stop();
    obj->reset();
    for(size_t i = 0; i < components.size(); ++i)
    {
        TIMEMORY_COMPONENT component = static_cast<TIMEMORY_COMPONENT>(components[i]);
        switch(component)
        {
            case WALL_CLOCK: obj->get<real_clock>() = new real_clock(); break;
            case SYS_CLOCK: obj->get<system_clock>() = new system_clock(); break;
            case USER_CLOCK: obj->get<user_clock>() = new user_clock(); break;
            case CPU_CLOCK: obj->get<cpu_clock>() = new cpu_clock(); break;
            case MONOTONIC_CLOCK:
                obj->get<monotonic_clock>() = new monotonic_clock();
                break;
            case MONOTONIC_RAW_CLOCK:
                obj->get<monotonic_raw_clock>() = new monotonic_raw_clock();
                break;
            case THREAD_CPU_CLOCK:
                obj->get<thread_cpu_clock>() = new thread_cpu_clock();
                break;
            case PROCESS_CPU_CLOCK:
                obj->get<process_cpu_clock>() = new process_cpu_clock();
                break;
            case CPU_UTIL: obj->get<cpu_util>() = new cpu_util(); break;
            case THREAD_CPU_UTIL:
                obj->get<thread_cpu_util>() = new thread_cpu_util();
                break;
            case PROCESS_CPU_UTIL:
                obj->get<process_cpu_util>() = new process_cpu_util();
                break;
            case CURRENT_RSS: obj->get<current_rss>() = new current_rss(); break;
            case PEAK_RSS: obj->get<peak_rss>() = new peak_rss(); break;
            case STACK_RSS: obj->get<stack_rss>() = new stack_rss(); break;
            case DATA_RSS: obj->get<data_rss>() = new data_rss(); break;
            case NUM_SWAP: obj->get<num_swap>() = new num_swap(); break;
            case NUM_IO_IN: obj->get<num_io_in>() = new num_io_in(); break;
            case NUM_IO_OUT: obj->get<num_io_out>() = new num_io_out(); break;
            case NUM_MINOR_PAGE_FAULTS:
                obj->get<num_minor_page_faults>() = new num_minor_page_faults();
                break;
            case NUM_MAJOR_PAGE_FAULTS:
                obj->get<num_major_page_faults>() = new num_major_page_faults();
                break;
            case NUM_MSG_SENT: obj->get<num_msg_sent>() = new num_msg_sent(); break;
            case NUM_MSG_RECV: obj->get<num_msg_recv>() = new num_msg_recv(); break;
            case NUM_SIGNALS: obj->get<num_signals>() = new num_signals(); break;
            case VOLUNTARY_CONTEXT_SWITCH:
                obj->get<voluntary_context_switch>() = new voluntary_context_switch();
                break;
            case PRIORITY_CONTEXT_SWITCH:
                obj->get<priority_context_switch>() = new priority_context_switch();
            case CUDA_EVENT:
#if defined(TIMEMORY_USE_CUDA)
                obj->get<cuda_event>() = new cuda_event();
#endif
                break;
        }
    }
    obj->push();
    obj->start();
    return obj;
}
