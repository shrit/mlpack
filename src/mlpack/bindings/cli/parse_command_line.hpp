/**
 * @file parse_command_line.hpp
 * @author Ryan Curtin
 * @author Matthew Amidon
 *
 * Parse the command line options.
 *
 * mlpack is free software; you may redistribute it and/or modify it under the
 * terms of the 3-clause BSD license.  You should have received a copy of the
 * 3-clause BSD license along with mlpack.  If not, see
 * http://www.opensource.org/licenses/BSD-3-Clause for more information.
 */
#ifndef MLPACK_BINDINGS_CMD_PARSE_COMMAND_LINE_HPP
#define MLPACK_BINDINGS_CMD_PARSE_COMMAND_LINE_HPP

#include <mlpack/core.hpp>
#include <boost/program_options.hpp>
#include "print_help.hpp"

#include <CLI/CLI.hpp>

namespace mlpack {
namespace bindings {
namespace cmd {

// Add default parameters that are included in every program.
// PARAM_FLAG("help", "Default help info.", "h");
PARAM_STRING_IN("info", "Print help on a specific option.", "", "");
PARAM_FLAG("verbose", "Display informational messages and the full list of "
     "parameters and timers at the end of execution.", "v");
PARAM_FLAG("version", "Display the version of mlpack.", "");

/**
 * Parse the command line, setting all of the options inside of the CMD object
 * to their appropriate given values.
 */
void ParseCommandLine(int argc, char** argv)
{
  // First, we need to build the boost::program_options variables for parsing.
  CLI::App app;

  // Go through list of options in order to add them.
  std::map<std::string, util::ParamData>& parameters = CMD::Parameters();
  std::map<std::string, std::string> boostNameMap;
  for (const auto it : parameters)
  {
    // Add the parameter to desc.
    const util::ParamData& d = it.second;
    CMD::GetSingleton().functionMap[d.tname]["AddToPO"](d, NULL,
        (void*) &app);

    // Generate the name the user passes on the command line.
    std::string boostName;
    CMD::GetSingleton().functionMap[d.tname]["MapParameterName"](d, NULL,
        (void*) &boostName);
    boostNameMap[boostName] = d.name;
  }

  // Mark that we did parsing.
  CMD::GetSingleton().didParse = true;

  // Parse the command line, then place the values in the right place.
  try
  {
    app.parse(argc, argv);

    // Iterate over all the options, looking for duplicate parameters.  If we
    // find any, remove the duplicates.  Note that vector options can have
    // duplicates, so we check for those with max_tokens().

    // I am not sure of the API names, do not check now
    // for (size_t i = 0; i < bpo.options.size(); ++i)
    // {
    //   for (size_t j = i + 1; j < bpo.options.size(); ++j)
    //   {
    //     if ((bpo.options[i].string_key == bpo.options[j].string_key) &&
    //         (desc.find(bpo.options[i].string_key,
    //                    false).semantic()->max_tokens() <= 1))
    //     {
    //       // If a duplicate is found, check to see if either one has a value.
    //       if (bpo.options[i].value.size() == 0 &&
    //           bpo.options[j].value.size() == 0)
    //       {
    //         // If neither has a value, we'll consider it a duplicate flag and
    //         // remove the duplicate.  It's important to not break out of this
    //         // loop because there might be another duplicate later on in the
    //         // vector.
    //         bpo.options.erase(bpo.options.begin() + j);
    //         --j; // Fix the index.
    //       }
    //       else
    //       {
    //         // If one or both has a value, produce an error and politely
    //         // terminate.  We pull the name from the original_tokens, rather
    //         // than from the string_key, because the string_key is the parameter
    //         // after aliases have been expanded.
    //         Log::Fatal << "\"" << bpo.options[j].original_tokens[0] << "\" is "
    //             << "defined multiple times." << std::endl;
    //       }
    //     }
    //   }
    // }

  }
  catch (const CLI::ParseError& pe)
  {
    Log::Fatal << "Caught exception from parsing command line: " << pe.what()
        << std::endl;
  }

  // Now iterate through the filled vmap, and overwrite default values with
  // anything that's found on the command line.

  std::cout << "App size: " << app.count_all() << std::endl;
  for (auto option : app.get_options()) 
  {
    std::cout << "Options: " << option->get_name() << std::endl;
   const std::string identifier = boostNameMap[option->get_name()];
    util::ParamData& param = parameters[identifier];
    param.wasPassed = true;
    // CMD::GetSingleton().functionMap[param.tname]["SetParam"](param,
    //     (void*) app.count(option->get_name()), NULL);
  }
  
  // for (std::size_t i = 0; i < app.count_all(); ++i)
  // {
  //   // There is not a possibility of an unknown option, since
  //   // boost::program_options would have already thrown an exception.  Because
  //   // some names may be mapped, we have to look through each ParamData object
  //   // and get its boost name.
 
  // }

  // If the user specified any of the default options (--help, --version, or
  // --info), handle those.

  // --version is prioritized over --help.
  if (CMD::HasParam("version"))
  {
    std::cout << CMD::GetSingleton().ProgramName() << ": part of "
        << util::GetVersion() << "." << std::endl;
    exit(0); // Don't do anything else.
  }

  // Default help message.
  if (CMD::HasParam("help"))
  {
    Log::Info.ignoreInput = false;
    PrintHelp();
    exit(0); // The user doesn't want to run the program, he wants help.
  }

  // Info on a specific parameter.
  if (CMD::HasParam("info"))
  {
    Log::Info.ignoreInput = false;
    std::string str = CMD::GetParam<std::string>("info");

    // The info node should always be there, but the user may not have specified
    // anything.
    if (str != "")
    {
      PrintHelp(str);
      exit(0);
    }

    // Otherwise just print the generalized help.
    PrintHelp();
    exit(0);
  }

  // Print whether or not we have debugging symbols.  This won't show anything
  // if we have not compiled in debugging mode.
  Log::Debug << "Compiled with debugging symbols." << std::endl;

  if (CMD::HasParam("verbose"))
  {
    // Give [INFO ] output.
    Log::Info.ignoreInput = false;
  }

  // Now, issue an error if we forgot any required options.
  for (std::map<std::string, util::ParamData>::const_iterator iter =
       parameters.begin(); iter != parameters.end(); ++iter)
  {
    const util::ParamData d = iter->second;
    if (d.required)
    {
      const std::string boostName;
      CMD::GetSingleton().functionMap[d.tname]["MapParameterName"](d, NULL,
          (void*) &boostName);

      if (!app.count(boostName))
      {
        Log::Fatal << "Required option --" << boostName << " is undefined."
            << std::endl;
      }
    }
  }
}

} // namespace cmd
} // namespace bindings
} // namespace mlpack

#endif
