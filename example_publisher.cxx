/*
* (c) Copyright, Real-Time Innovations, 2020.  All rights reserved.
* RTI grants Licensee a license to use, modify, compile, and create derivative
* works of the software solely for use with RTI Connext DDS. Licensee may
* redistribute copies of the software provided that all such copies are subject
* to this license. The software is provided "as is", with no warranty of any
* type, including any warranty for fitness for any purpose. RTI is under no
* obligation to maintain or support the software. RTI shall not be liable for
* any incidental or consequential damages arising out of the use or inability
* to use the software.
*/

#include <iostream>

#include <rti/rti.hpp>

#include "application.hpp"  // for command line parsing and ctrl-c
#include "example.hpp"

// Listener that will be notified of DataWriter events
class MyTypeDataWriterListener : public dds::pub::NoOpDataWriterListener<MyType> {
  public:
    // Notifications about data
    void on_offered_deadline_missed(
        dds::pub::DataWriter<MyType>& writer,
        const dds::core::status::OfferedDeadlineMissedStatus& status)
        override
    {
    }
    // Notifications about DataReaders
    void on_offered_incompatible_qos(
        dds::pub::DataWriter<MyType>& writer,
        const dds::core::status::OfferedIncompatibleQosStatus& status)
        override
    {
    }
    void on_publication_matched(
        dds::pub::DataWriter<MyType>& writer,
        const ::dds::core::status::PublicationMatchedStatus& status)
        override
    {
    }
    // Notification about DataWriter's liveliness
    void on_liveliness_lost(
        dds::pub::DataWriter<MyType>& writer,
        const dds::core::status::LivelinessLostStatus& status) override
    {
    }
};

void run_publisher_application(unsigned int domain_id, unsigned int sample_count)
{
    dds::domain::DomainParticipant participant(
            domain_id,
            dds::core::QosProvider::Default().participant_qos("example_Library::example_Profile"));
    dds::topic::Topic<MyType> topic(participant, "Example MyType");
    dds::pub::Publisher publisher(participant);

    // Enable the DataWriter statuses we want to be notified about 
    dds::core::status::StatusMask status_mask;
    status_mask |= dds::core::status::StatusMask::offered_deadline_missed()
            | dds::core::status::StatusMask::offered_incompatible_qos()
            | dds::core::status::StatusMask::publication_matched()
            | dds::core::status::StatusMask::liveliness_lost();

    auto listener = std::make_shared<MyTypeDataWriterListener>();
    dds::pub::DataWriter<MyType> writer(
            publisher,
            topic,
            dds::core::QosProvider::Default().datawriter_qos("example_Library::example_Profile"),
            listener,
            status_mask);

    MyType data;
    for (unsigned int samples_written = 0;
        !application::shutdown_requested && samples_written < sample_count;
        samples_written++) {

        data.id(1); // simple, arbitrary number for the ID
        data.value(static_cast<int32_t>(samples_written));

        std::cout << "Writing MyType, count " << samples_written << std::endl;

        writer.write(data);

        // Send every 1 second
        rti::util::sleep(dds::core::Duration(1));
    }
}

int main(int argc, char *argv[])
{

    using namespace application;

    // Parse arguments and handle control-C
    auto arguments = parse_arguments(argc, argv);
    if (arguments.parse_result == ParseReturn::exit) {
        return EXIT_SUCCESS;
    } else if (arguments.parse_result == ParseReturn::failure) {
        return EXIT_FAILURE;
    }
    setup_signal_handlers();

    rti::config::Logger::instance().verbosity(arguments.verbosity);

    try {
        run_publisher_application(arguments.domain_id, arguments.sample_count);
    } catch (const std::exception& ex) {
        std::cerr << "Exception in run_publisher_application(): " << ex.what()
        << std::endl;
        return EXIT_FAILURE;
    }
    
    dds::domain::DomainParticipant::finalize_participant_factory();
    return EXIT_SUCCESS;
}
