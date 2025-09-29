/**
 *
 * @file interrupts.cpp
 * @author Khadija Lahlou
 * @author Casey 
 *
 */

#include<interrupts.hpp>
// if it fails #include "interrupts.hpp"

// to run ./interrupts test /test/ <your_vector_table.txt> <your_device_table.txt>

int main(int argc, char** argv) {

    //vectors is a C++ std::vector of strings that contain the address of the ISR
    //delays  is a C++ std::vector of ints that contain the delays of each device
    //the index of these elemens is the device number, starting from 0
    auto [vectors, delays] = parse_args(argc, argv);
    std::ifstream input_file(argv[1]);

    std::string trace;      //!< string to store single line of trace file
    std::string execution;  //!< string to accumulate the execution output

    /******************ADD YOUR VARIABLES HERE*************************/



    /******************************************************************/

    //parse each line of the input trace file
    while(std::getline(input_file, trace)) {
        auto [activity, duration_intr] = parse_trace(trace);

        /******************ADD YOUR SIMULATION CODE HERE*************************/

        if (activity == "CPU"){
            // 5 lines of code
        }
        else if(activity == "SYSCALL") {
            // 5 lines of code
        }
        else if (activity == "END_IO") {
            // 5 lines of code
        }
        else {
            std::cerr << "Error: Unknown activity type: " << activity << std::endl;
       
        }        
        

        /************************************************************************/

    }
    //Test
    input_file.close();

    write_output(execution);

    return 0;
}
