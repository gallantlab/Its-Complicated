#include "MRIExperimentState.h"

using namespace std;

MRIExperimentState::MRIExperimentState(double time)
	: isTTL(false),
	  time(time),
	  frame(-1),
	  points(0)
{
	entityStates = TArray<EntityState>();
}

MRIExperimentState::MRIExperimentState(double time, int frame)
	: isTTL(false),
	  time(time),
	  frame(frame),
	  points(0)
{
	entityStates = TArray<EntityState>();
}

void MRIExperimentState::Record(TArray<const UMRIAgentComponent*> agents, AMRISubjectState* playerState)
{
	RecordStates(agents);
	totalTTLs = playerState->GetTotalTTLs();
	displayedPromptType = playerState->GetDisplayedPromptType();
	points = playerState->GetPoints();
}

void MRIExperimentState::RecordStates(TArray<const UMRIAgentComponent*> agents)
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

void MRIExperimentState::Visit(const UMRIAgentComponent& agent)
{
	// Fatal log: deliberately crashes in all build configurations, carrying the
	// message into the crash report. An unimplemented Visit() must not go unnoticed.
	UE_LOG(LogMRI, Fatal, TEXT("No Visit() override for agent component class %s."), *agent.GetClass()->GetName());
}

void MRIExperimentState::Visit(const UMRIPlayerAgentComponent& agent)
{
	UE_LOG(LogMRI, Fatal, TEXT("Visit() not overridden for the player agent component."));
}

void MRIExperimentState::WriteToFile(ofstream& logFile, int& indentation)
{
	MRI_WRITE_INDENTS(logFile, indentation);
	logFile << "<Frame"
		<< MRI_WRITE_ATTRIBUTE_KV("Number", frame)
		<< MRI_WRITE_ATTRIBUTE_KV("Time", time)
		<< MRI_WRITE_ATTRIBUTE_KV("TTL", isTTL)
		<< MRI_WRITE_ATTRIBUTE_KV("TotalTTL", totalTTLs)
		<< MRI_WRITE_ATTRIBUTE_KV("Points", points)
		<< ">" << endl;

	indentation++;
	WriteContents(logFile, indentation);
	indentation--;

	MRI_WRITE_INDENTS(logFile, indentation);
	logFile << "</Frame>" << endl;
}

void MRIExperimentState::WriteContents(ofstream& stream, int& indent)
{
	MRI_WRITE_INDENTS(stream, indent);
	stream << "<Player"
		<< MRI_WRITE_ATTRIBUTE_KV("Prompt", (int)displayedPromptType)
		<< " />" << endl;

	for (int i = 0; i < entityStates.Num(); i++)
	{
		MRI_WRITE_INDENTS(stream, indent);
		stream << "<Entity"
			<< MRI_WRITE_ATTRIBUTE_KV("Type", (int)entityStates[i].type)
			<< MRI_WRITE_ATTRIBUTE_KV("ID", entityStates[i].ID)
			<< ">" << endl;

		indent++;

		MRI_WRITE_INDENTS(stream, indent);
		stream << "<Position"
			<< MRI_WRITE_ATTRIBUTE_KV("X", entityStates[i].position.X)
			<< MRI_WRITE_ATTRIBUTE_KV("Y", entityStates[i].position.Y)
			<< MRI_WRITE_ATTRIBUTE_KV("Z", entityStates[i].position.Z)
			<< " />" << endl;

		MRI_WRITE_INDENTS(stream, indent);
		stream << "<Rotation"
			<< MRI_WRITE_ATTRIBUTE_KV("Pitch", entityStates[i].rotation.Pitch)
			<< MRI_WRITE_ATTRIBUTE_KV("Roll", entityStates[i].rotation.Roll)
			<< MRI_WRITE_ATTRIBUTE_KV("Yaw", entityStates[i].rotation.Yaw)
			<< " />" << endl;

		indent--;

		MRI_WRITE_INDENTS(stream, indent);
		stream << "</Entity>" << endl;
	}
}

MRIExperimentState::~MRIExperimentState()
{
}
