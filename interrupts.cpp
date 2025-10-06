/**
 *
 * @file interrupts.cpp
 * 
 * @author Khadija Lahlou
 * @author Casey Ramanampanoharana
 * @date 2025-10-03
 * 
 */

#include "interrupts.hpp"

#define CONTEXT_SAVE_EXECUTION_TIME 10 // in ms
#define ISR_EXECUTION_TIME 40 // in ms
#define IRET_EXECUTION_TIME 1 // in ms
#define NOT_DEVICE -1

// quick note: if you get an error here
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

    int current_time = 0;
    int current_device = NOT_DEVICE;

    // Capital variables are constants

    
    /******************************************************************/

    //parse each line of the input trace file
    while(std::getline(input_file, trace)) {
        auto [activity, duration_intr] = parse_trace(trace);

        /******************ADD YOUR SIMULATION CODE HERE*************************/
        if (activity == "CPU")
        {
            // Context:
                //   CPU, 51 <-- We are here
                //   SYSCALL, 14
                //   CPU, 39
                //   END_IO, 14
                //
                // Added IRET to make it obvious the return to CPU
                // This is the first CPU instruction --> Will be considered as a CPU Burst can be initialize_variable() or calculate(x) or delete (y,x)
                execution += std::to_string(current_time) + ", " + std::to_string(duration_intr) + ", CPU Burst\n";

                current_time += duration_intr;

            continue;
        }
        else if (activity == "SYSCALL")
        {
            // Context: 
            //   CPU, 51
            //   SYSCALL, 14 <-- We are here
            //   CPU, 39
            //   END_IO, 14
            //  
            // Device delays --> Total from ISR, +40 every step, from example execution file from github 2 ISR :
            // transfer data from device to memory, check for errors
            current_device = duration_intr;

            auto [boilerplate_execution, new_current_time] = intr_boilerplate(current_time, current_device, CONTEXT_SAVE_EXECUTION_TIME, vectors);
            execution += boilerplate_execution;

            current_time = new_current_time;

            execution += std::to_string(current_time) + ", " + std::to_string(ISR_EXECUTION_TIME) + ", SYSCALL: run the ISR (device driver)\n";
            current_time += ISR_EXECUTION_TIME;
            
            execution += std::to_string(current_time) + ", " + std::to_string(ISR_EXECUTION_TIME) + ", transfer data from device to memory\n";
            current_time += ISR_EXECUTION_TIME;

            execution += std::to_string(current_time) + ", " + std::to_string(delays.at(current_device)-(2*ISR_EXECUTION_TIME)) + ", check for errors\n";
            current_time += delays.at(current_device)-(2*ISR_EXECUTION_TIME);

            execution += std::to_string(current_time) + ", " + std::to_string(IRET_EXECUTION_TIME) + ", IRET \n";
            current_time += IRET_EXECUTION_TIME;

            continue;
        }
        else if (activity == "END_IO")
        {
            // Context: 
            //   SYSCALL, 14
            //   CPU, 39
            //   END_IO, 14 <-- We are here
            //
            // Added IRET to make it obvious the return to CPU
            // Device delays --> Total from ISR, +40 every step, from example execution file from github 1 ISR : check device status
            current_device = duration_intr;

            auto [boilerplate_execution, new_current_time] = intr_boilerplate(current_time, current_device, CONTEXT_SAVE_EXECUTION_TIME, vectors);
            execution += boilerplate_execution;

            current_time = new_current_time;

            execution += std::to_string(current_time) + ", " + std::to_string(ISR_EXECUTION_TIME) + ", end of I/O " + std::to_string(current_device) + ": run the ISR (device driver)\n";
            current_time += ISR_EXECUTION_TIME;

            execution += std::to_string(current_time) + ", " + std::to_string(delays.at(current_device)-ISR_EXECUTION_TIME) + " check device status\n";
            current_time += delays.at(current_device)-ISR_EXECUTION_TIME;

            execution += std::to_string(current_time) + ", " + std::to_string(IRET_EXECUTION_TIME) + ", IRET \n";
            current_time += IRET_EXECUTION_TIME;

            current_device = NOT_DEVICE;

            continue;
        }
        /************************************************************************/

    }

    input_file.close();

    write_output(execution);

    return 0;
}
