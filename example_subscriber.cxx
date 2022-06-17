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

#include <algorithm>
#include <iostream>

#include <rti/rti.hpp>

#include "example.hpp"
#include "application.hpp"  // for command line parsing and ctrl-c

// Listener that will be notified of DataReader events
class MyTypeDataReaderListener : public dds::sub::NoOpDataReaderListener<MyType> {
  public:
    // Notifications about data
    void on_requested_deadline_missed(
        dds::sub::DataReader<MyType>& reader,
        const dds::core::status::RequestedDeadlineMissedStatus& status)
        override 
    {
    }
    void on_sample_rejected(
        dds::sub::DataReader<MyType>& reader,
        const dds::core::status::SampleRejectedStatus& status) override
    {
    }
    void on_sample_lost(
        dds::sub::DataReader<MyType>& reader,
        const dds::core::status::SampleLostStatus& status) override
    {
    } 
    // Notifications about DataWriters
    void on_requested_incompatible_qos(
        dds::sub::DataReader<MyType>& reader,
        const dds::core::status::RequestedIncompatibleQosStatus& status)
        override
    {
    }
    void on_subscription_matched(
        dds::sub::DataReader<MyType>& reader,
        const dds::core::status::SubscriptionMatchedStatus& status) override
    {
    }
    void on_liveliness_changed(
        dds::sub::DataReader<MyType>& reader,
        const dds::core::status::LivelinessChangedStatus& status) override
    {
    }
};

int process_data(dds::sub::DataReader<MyType> reader)
{
    // Take all samples
    int count = 0;
    dds::sub::LoanedSamples<MyType> samples = reader.take();
    for (const auto& sample : samples) {
        if (sample.info().valid()) {
            count++;
            if (sample.info().extensions().coherent_set_info().is_set()) {
                std::cout << "INFO: coherent_set_sequence_number = " 
                        << sample.info().extensions().coherent_set_info().value().coherent_set_sequence_number() 
                        << std::endl;              
            }
            std::cout << sample.data() << std::endl;
        } else {
            std::cout << "Instance state changed to "
            << sample.info().state().instance_state() << std::endl;
        }
    }

    return count; 
} // The LoanedSamples destructor returns the loan

void run_subscriber_application(unsigned int domain_id, unsigned int sample_count)
{
    dds::domain::DomainParticipant participant(
            domain_id,
            dds::core::QosProvider::Default().participant_qos("example_Library::example_Profile"));

    dds::topic::Topic<MyType> topic(participant, example_topic_name);

    dds::sub::Subscriber subscriber(participant, dds::core::QosProvider::Default().subscriber_qos("example_Library::example_Profile"));

    // Configure the status mask such that the DataReader's listener will handle
    // all status changes *except* for on_data_available... we'll get notified
    // of new samples via a ReadCondition instead.
    auto status_mask = dds::core::status::StatusMask::all()
            & ~dds::core::status::StatusMask::data_available();

    auto listener = std::make_shared<MyTypeDataReaderListener>();
    dds::sub::DataReader<MyType> reader(
            subscriber,
            topic,
            dds::core::QosProvider::Default().datareader_qos("example_Library::example_Profile"),
            listener,
            status_mask);

    // Create a ReadCondition for any data received on this reader and set a
    // handler to process the data
    unsigned int samples_read = 0;
    dds::sub::cond::ReadCondition read_condition(
        reader,
        dds::sub::status::DataState::any(),
        [reader, &samples_read]() { samples_read += process_data(reader); });

    // WaitSet will be woken when the attached condition is triggered
    dds::core::cond::WaitSet waitset;
    waitset += read_condition;

    while (!application::shutdown_requested && samples_read < sample_count) {
        std::cout << "MyType subscriber sleeping up to 1 sec..." << std::endl;

        // Run the handlers of the active conditions. Wait for up to 1 second.
        waitset.dispatch(dds::core::Duration(1));
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

    // Sets Connext verbosity to help debugging
    rti::config::Logger::instance().verbosity(arguments.verbosity);

    try {
        run_subscriber_application(arguments.domain_id, arguments.sample_count);
    } catch (const std::exception& ex) {
        // This will catch DDS exceptions
        std::cerr << "Exception in run_subscriber_application(): " << ex.what()
        << std::endl;
        return EXIT_FAILURE;
    }

    // Releases the memory used by the participant factory.  Optional at
    // application exit
    dds::domain::DomainParticipant::finalize_participant_factory();

    return EXIT_SUCCESS;
}
