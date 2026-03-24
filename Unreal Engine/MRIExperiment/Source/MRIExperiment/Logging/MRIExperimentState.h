#pragma once

#include "CoreMinimal.h"
#include "Agent/MRIAgentComponent.h"
#include "Agent/MRIAgentComponentVisitor.h"
#include "State/MRISubjectState.h"

#include <iostream>
#include <fstream>

#ifndef MRI_WRITE_INDENTS
#define MRI_WRITE_INDENTS(out, indents) for (int indent = 0; indent < indents; indent++) out << '\t';
#endif

#ifndef MRI_WRITE_ATTRIBUTE
#define MRI_WRITE_ATTRIBUTE(attribute) '"' << attribute << '"'
#endif

#define MRI_WRITE_ATTRIBUTE_KV(key, value) " " << key << "=\"" << value << '"'

#define MRI_WRITE_SINGLE_TAG(name, content) "<" << name << ">" << content << "</" << name << ">"


struct EntityState
{
	EntityState(FVector position, FRotator rotation, EntityType type, uint32 ID)
		: position(position),
		  rotation(rotation),
		  type(type),
		  ID(ID)
	{
	}

	FVector position;
	FRotator rotation;
	EntityType type;
	uint32 ID;
};

class MRIExperimentState : public IMRIAgentComponentVisitor
{
public:

	MRIExperimentState(double time);
	MRIExperimentState(double time, int frame);
	MRIExperimentState(double time, int frame, TArray<const UMRIAgentComponent*> agents);
	MRIExperimentState(double time, int frame, TArray<const UMRIAgentComponent*> agents, AMRISubjectState* playerState);

	void RecordStates(TArray<const UMRIAgentComponent*> agents);

	double GetFrameTime() const { return time; }
	int GetFrame() const { return frame; }
	int TotalTTLCount() const { return totalTTLs; }
	EDisplayedPromptType GetDisplayedPromptType() const { return displayedPromptType; }
	bool IsBeep() const { return isBeep; }
	int GetPoints() const { return points; }

	// and add override for other agent types
	virtual void Visit(const UMRIAgentComponent& agent) override;

	EntityState& operator[](int index) { return entityStates[index]; }
	int Num() const { return entityStates.Num(); }

	bool isTTL;

	void WriteToFile(std::ofstream& logFile, int& indentation);

	~MRIExperimentState();

protected:

	virtual void WriteContents(std::ofstream& stream, int& indent);

private:

	double time;
	int frame;
	EntityType thisType;
	TArray<EntityState> entityStates;

	int totalTTLs = 0;
	bool isBeep;
	int points = 0;
	EDisplayedPromptType displayedPromptType;
};
