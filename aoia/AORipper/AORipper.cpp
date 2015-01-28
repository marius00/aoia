// AORipper.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/assign.hpp>
#include <iostream>
#include <Shared/AODatabaseParser.h>
#include <Shared/AODatabaseWriter.h>
#include <Shared/AODatabaseIndex.h>


namespace bpo = boost::program_options;
namespace bfs = boost::filesystem;

// Forwards
bool RippIt(std::string const& input, std::string const& output, bool doPostProcess);


int _tmain(int argc, _TCHAR* argv[])
{
    std::string input;
    std::string output;
    bool doPostProcess;
    bool doForce; 

    // Declare the supported options.
    bpo::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "produce help message")
        ("input,i", bpo::value<std::string>(&input), "Specify the path to the AO folder.")
        ("output,o", bpo::value<std::string>(&output), "Specify the output SQLite file.")
        ("postprocess,p", bpo::value<bool>(&doPostProcess)->default_value(false), "Specify if postprocessing should be done or not.")
        ("force,f", bpo::value<bool>(&doForce)->default_value(false), "Force overwriting the output file if it exists.")
        ;

    bpo::variables_map vm;
    bpo::store(bpo::parse_command_line(argc, argv, desc), vm);
    bpo::notify(vm);    

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }

    if (input.empty() || output.empty()) {
        std::cerr << "Invalid arguments.\n";
        std::cout << desc << "\n";
        return 2;
    }

    if (!bfs::exists(input)) {
        std::cerr << "Could not locate input folder.";
        return 3;
    }
    if (bfs::exists(output)) {
        if (!doForce) {
            std::cerr << "Output file already exists.";
            return 3;
        }
        bfs::remove(output);
    }

    if (!RippIt(input, output, doPostProcess)) {
        return 4;
    }

    return 0;
}


bool RippIt(std::string const& input_folder, std::string const& output_file, bool doPostProcess)
{
    try
    {
        // Set up indexer
        std::set<ResourceType> resource_types = boost::assign::list_of(AODB_TYP_ITEM)(AODB_TYP_NANO);
        AODatabaseIndex indexer(input_folder + "/cd_image/data/db/ResourceDatabase.idx", resource_types);
        std::vector<unsigned int> item_offsets = indexer.GetOffsets(AODB_TYP_ITEM);
        std::vector<unsigned int> nano_offsets = indexer.GetOffsets(AODB_TYP_NANO);

        // Set up parser
        std::vector<std::string> original_files = boost::assign::list_of
            (input_folder + "/cd_image/data/db/ResourceDatabase.dat")
            (input_folder + "/cd_image/data/db/ResourceDatabase.dat.001");
        AODatabaseParser aodb(original_files);

        // Set up writer
        AODatabaseWriter writer(output_file, std::cout);

        // Extract items
        unsigned int itemcount = 0;
        writer.BeginWrite();
        for (std::vector<unsigned int>::iterator item_it = item_offsets.begin(); item_it != item_offsets.end(); ++item_it)
        {
            boost::shared_ptr<ao_item> item = aodb.GetItem(*item_it);
            ++itemcount; 
            if (item)
            {
                writer.WriteItem(item);
                if (itemcount % 10000 == 0) {
                    writer.CommitItems();
                    writer.BeginWrite();
                }
            }
        }
        writer.CommitItems();
        std::cout << "Parsed " << itemcount << " item records." << std::endl;

        // Extract nano programs
        itemcount = 0;
        writer.BeginWrite();
        for (std::vector<unsigned int>::iterator nano_it = nano_offsets.begin(); nano_it != nano_offsets.end(); ++nano_it)
        {
            boost::shared_ptr<ao_item> nano = aodb.GetItem(*nano_it);
            ++itemcount; 
            if (nano)
            {
                writer.WriteItem(nano);
                if (itemcount % 10000 == 0) {
                    writer.CommitItems();
                    writer.BeginWrite();
                }
            }
        }
        writer.CommitItems();
        std::cout << "Parsed " << itemcount << " nano records." << std::endl;

        if (doPostProcess)
        {
            std::cout << "Post processing." << std::endl;
            writer.PostProcessData();
        }
    }
    catch (AODatabaseParser::Exception &e)
    {
        assert(false);
        std::cerr << "Error parsing: " << e.what();
        return false;
    }
    catch (std::bad_alloc &e) {
        assert(false);
        std::cerr << "Error creating item database. " << e.what();
        return false;
    }
    catch (std::exception &e) {
        assert(false);
        std::cerr << "Error creating item database. " << e.what();
        return false;
    }

    return true;
}