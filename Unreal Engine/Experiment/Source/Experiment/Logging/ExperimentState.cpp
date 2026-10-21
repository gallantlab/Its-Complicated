#include "ExperimentState.h"

using namespace std;

ExperimentState::ExperimentState(double time)
	: isTTL(false),
	  time(time),
	  frame(-1),
	  points(0)
{
	entityStates = TArray<EntityState>();
}

ExperimentState::ExperimentState(double time, int frame)
	: isTTL(false),
	  time(time),
	  frame(frame),
	  points(0)
{
	entityStates = TArray<EntityState>();
}

void ExperimentState::Record(TArray<const UExperimentAgentComponent*> agents, AExperimentSubjectState* playerState)
{
	RecordStates(agents);
	totalTTLs = playerState->GetTotalTTLs();
	displayedPromptType = playerState->GetDisplayedPromptType();
	points = playerState->GetPoints();
}

void ExperimentState::RecordStates(TArray<const UExperimentAgentComponent*> agents)
{
	for (int i = 0; i < agents.Num(); i++)
	{
		agents[i]->AcceptVisitor(*this);
		FVector position = agents[i]->GetComponentLocation();
		FRotator rotation = agents[i]->GetComponentRotation();
		uint32 agentID = agents[i]->GetId();
		entityStates.Add(EntityState(position, rotation, thisType, agentID));
	}
}

void ExperimentState::Visit(const UExperimentAgentComponent& agent)
{
	// Fatal log: deliberately crashes in all build configurations, carrying the
	// message into the crash report. An unimplemented Visit() must not go unnoticed.
	UE_LOG(LogExperiment, Fatal, TEXT("No Visit() override for agent component class %s."), *agent.GetClass()->GetName());
}

void ExperimentState::Visit(const UExperimentPlayerAgentComponent& agent)
{
	UE_LOG(LogExperiment, Fatal, TEXT("Visit() not overridden for the player agent component."));
}

void ExperimentState::WriteToFile(ofstream& logFile, int& indentation)
{
	Experiment_WRITE_INDENTS(logFile, indentation);
	logFile << "<Frame"
		<< Experiment_WRITE_ATTRIBUTE_KV("Number", frame)
		<< Experiment_WRITE_ATTRIBUTE_KV("Time", time)
		<< Experiment_WRITE_ATTRIBUTE_KV("TTL", isTTL)
		<< Experiment_WRITE_ATTRIBUTE_KV("TotalTTL", totalTTLs)
		<< Experiment_WRITE_ATTRIBUTE_KV("Points", points)
		<< ">" << endl;

	indentation++;
	WriteContents(logFile, indentation);
	indentation--;

	Experiment_WRITE_INDENTS(logFile, indentation);
	logFile << "</Frame>" << endl;
}

void ExperimentState::WriteContents(ofstream& stream, int& indent)
{
	Experiment_WRITE_INDENTS(stream, indent);
	stream << "<Player"
		<< Experiment_WRITE_ATTRIBUTE_KV("Prompt", (int)displayedPromptType)
		<< " />" << endl;

	for (int i = 0; i < entityStates.Num(); i++)
	{
		Experiment_WRITE_INDENTS(stream, indent);
		stream << "<Entity"
			<< Experiment_WRITE_ATTRIBUTE_KV("Type", (int)entityStates[i].type)
			<< Experiment_WRITE_ATTRIBUTE_KV("ID", entityStates[i].ID)
			<< ">" << endl;

		indent++;

		Experiment_WRITE_INDENTS(stream, indent);
		stream << "<Position"
			<< Experiment_WRITE_ATTRIBUTE_KV("X", entityStates[i].position.X)
			<< Experiment_WRITE_ATTRIBUTE_KV("Y", entityStates[i].position.Y)
			<< Experiment_WRITE_ATTRIBUTE_KV("Z", entityStates[i].position.Z)
			<< " />" << endl;

		Experiment_WRITE_INDENTS(stream, indent);
		stream << "<Rotation"
			<< Experiment_WRITE_ATTRIBUTE_KV("Pitch", entityStates[i].rotation.Pitch)
			<< Experiment_WRITE_ATTRIBUTE_KV("Roll", entityStates[i].rotation.Roll)
			<< Experiment_WRITE_ATTRIBUTE_KV("Yaw", entityStates[i].rotation.Yaw)
			<< " />" << endl;

		indent--;

		Experiment_WRITE_INDENTS(stream, indent);
		stream << "</Entity>" << endl;
	}
}

ExperimentState::~ExperimentState()
{
}
