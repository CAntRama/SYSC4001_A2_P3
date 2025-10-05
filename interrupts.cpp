/**
 *
 * @file interrupts.cpp
 * @author Sasisekhar Govind
 *
 */

#include<interrupts.hpp>

#define CONTEXT_SAVE_EXECUTION_TIME 10 // in ms
#define IRET_EXECUTION_TIME 1 // in ms
#define NOT_DEVICE -1

int main(int argc, char** argv) {

    //vectors is a C++ std::vector of strings that contain the address of the ISR
    //delays  is a C++ std::vector of ints that contain the delays of each device
    //the index of these elemens is the device number, starting from 0
    auto [vectors, delays] = parse_args(argc, argv);
    std::ifstream input_file(argv[1]);

    std::string trace;      //!< string to store single line of trace file
    std::string execution;  //!< string to accumulate the execution output

    /******************ADD YOUR VARIABLES HERE*************************/
    std::array<std::string, 2> cpu_operations = {"calculate(x)", "delete(y, x)"};

    int current_time = 0;
    bool variables_initialized = false;
    bool within_syscall = false;
    int current_device = NOT_DEVICE;
    int next_cpu_operation = 0; // next operation is "calulate"


    /******************************************************************/

    //parse each line of the input trace file
    while(std::getline(input_file, trace)) {
        auto [activity, duration_intr] = parse_trace(trace);

        /******************ADD YOUR SIMULATION CODE HERE*************************/
        if (activity == "CPU")
        {
            if (variables_initialized == false)
            {
                // Context:
                //   CPU, 51 <-- We are here
                //   SYSCALL, 14
                //
                // This is the first CPU instruction in the file and we assume that it's the initialize_variabes()
                execution += std::to_string(current_time) + ", " + std::to_string(duration_intr) + ", initialize_variabes()\n";

                variables_initialized = true;

                current_time += duration_intr;
            }
            else if (within_syscall)
            {
                // Context:
                //   SYSCALL, 14
                //   CPU, 39 <-- We are here
                //   END_IO, 14
                //
                // What is this CPU instruction exactly?
                //   1. Is this the "Calling device driver?"
                //      1. Is the duration the time that the driver took to execute?    
                //   2. Is it something else?
                //
                // In the example:
                //   595, 24, call device driver // include all activities of the ISR (takes 110ms according to device table)
                //   ...
                //   705, 1, IRET // IRET after 110ms
                // 
                //   1. What is 24? Is the duration that comes wit the CPU instruction?
                //   2. If that is the case, why is the IRET at 705 (595+110) and not 729 (595+24+110)
                //
                // And does the "..." mean something?
                //  1. Are we supposed to fill it in with something?
                //     1. With what?
                //  2. Are we supposed to just write "..." as well?
                //  3. For now, we're not writing anything about it.
                execution += std::to_string(current_time) + ", " + std::to_string(duration_intr) + ", Calling device driver\n";

                // For now, we're going to assume that:
                //   1. This CPU instruction is the "Calling device driver"
                //   2. Total duration = CPU duration + Device delay
                current_time += duration_intr + delays.at(current_device);
            }
            else
            {
                // Context:
                //   END_IO, 14
                //   CPU, 72 <-- We are here
                //
                // We're in one of the non initialize CPU operations: calculate or delete
                execution += std::to_string(current_time) + ", " + std::to_string(duration_intr) + ", " + cpu_operations[next_cpu_operation] + "\n";

                next_cpu_operation++;
                if (next_cpu_operation >= cpu_operations.size())
                {
                    next_cpu_operation = 0;
                }

                current_time += duration_intr;
            }

            continue;
        }
        else if (activity == "SYSCALL")
        {
            // Context: 
            //   SYSCALL, 14 <-- We are here
            //   CPU, 39
            //   END_IO, 14
            current_device = duration_intr;

            auto [boilerplate_execution, new_current_time] = intr_boilerplate(current_time, current_device, CONTEXT_SAVE_EXECUTION_TIME, vectors);
            execution += boilerplate_execution;

            current_time = new_current_time;

            within_syscall = true;

            continue;
        }
        else if (activity == "END_IO")
        {
            // Context: 
            //   SYSCALL, 14
            //   CPU, 39
            //   END_IO, 14 <-- We are here
            execution += std::to_string(current_time) + ", " + std::to_string(IRET_EXECUTION_TIME) + ", IRET\n";
            current_time += IRET_EXECUTION_TIME;

            execution += std::to_string(current_time) + ", " + std::to_string(duration_intr) + ", end of I/O " + std::to_string(current_device) + ": interrupt\n";
            current_time += duration_intr;

            within_syscall = false;
            current_device = NOT_DEVICE;

            continue;
        }
        /************************************************************************/

    }
    //Test2
    input_file.close();

    write_output(execution);

    return 0;
}
