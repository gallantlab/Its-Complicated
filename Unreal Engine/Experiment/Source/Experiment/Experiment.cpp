// Experiment Plugin

#include "Experiment.h"

DEFINE_LOG_CATEGORY(LogExperiment);

IMPLEMENT_MODULE(FExperimentModule, Experiment)

void FExperimentModule::StartupModule()
{
	UE_LOG(LogExperiment, Log, TEXT("Experiment module loaded"));
}

void FExperimentModule::ShutdownModule()
{
	UE_LOG(LogExperiment, Log, TEXT("Experiment module unloaded"));
}
