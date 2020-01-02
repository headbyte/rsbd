#include <iostream>
#include <args.hxx>

using namespace std;

#include "single_file_block_storage.h"
#include "block_storage_sync.h"

using namespace rsbd;

void create_command(args::Subparser &parser) {
    args::ValueFlag<std::string> path(parser, "path", "Path for block disk.", {"path"}, args::Options::Required);
    args::ValueFlag<block_size_t> block_size(parser, "block-size", "Size of blocks for the block disk.", {"block-size"},
                                             args::Options::Required);
    args::ValueFlag<block_id_t> block_count(parser, "block-count", "Number of blocks for the block disk.",
                                            {"block-count"}, args::Options::Required);
    parser.Parse();

    std::cout << "Creating a new block disk " << std::endl <<
              "Path        : " << path.Get() << std::endl <<
              "Block size  : " << block_size.Get() << std::endl <<
              "Block count : " << block_count.Get() << std::endl;

    single_file_block_storage sfbs;
    sfbs.create(path.Get(), block_size.Get(), block_count.Get());
}


void info_command(args::Subparser &parser) {
    args::ValueFlag<std::string> path(parser, "path", "Path for block disk.", {"path"}, args::Options::Required);
    parser.Parse();


    std::cout << "Opening block disk " << std::endl;

    single_file_block_storage sfbs;
    sfbs.open(path.Get());
    std::cout <<
              "Path        : " << path.Get() << std::endl <<
              "Block size  : " << sfbs.get_block_size(0) << std::endl <<
              "Block count : " << sfbs.get_block_count() << std::endl;
}


void sync_command(args::Subparser &parser) {
    args::ValueFlag<std::string> from(parser, "from", "Path for source block disk.", {"from"}, args::Options::Required);
    args::ValueFlag<std::string> to(parser, "to", "Path for destination block disk.", {"to"}, args::Options::Required);
    parser.Parse();


    std::cout << "Synchronising block disks " << std::endl;

    single_file_block_storage from_bd;
    from_bd.open(from.Get());
    std::cout <<
              "From Path   : " << from.Get() << std::endl <<
              "Block size  : " << from_bd.get_block_size(0) << std::endl <<
              "Block count : " << from_bd.get_block_count() << std::endl;

    single_file_block_storage to_bd;
    to_bd.open(to.Get());
    std::cout <<
              "To Path     : " << to.Get() << std::endl <<
              "Block size  : " << to_bd.get_block_size(0) << std::endl <<
              "Block count : " << to_bd.get_block_count() << std::endl;

    std::cout << "Synchronising..." << std::endl;

    block_storage_sync sync(from_bd, to_bd);
    sync.synchronise();

    std::cout << "Synchronisation complete!" << std::endl;
}


int main(int argc, char **argv) {


    args::ArgumentParser p("rsbd");
    args::Group commands(p, "commands");
    args::Command create(commands, "create", "add file contents to the index", &create_command);
    args::Command info(commands, "info", "reads header information from a block disk file and prints", &info_command);
    args::Command sync(commands, "sync", "synchronises two block disks", &sync_command);

    try {
        p.ParseCLI(argc, argv);
    }
    catch (args::Help) {
        std::cout << p;
    }
    catch (args::Error &e) {
        std::cerr << e.what() << std::endl << p;
        return 1;
    }
    return 0;

}
