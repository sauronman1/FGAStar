#include "FGGrid.h"

// Sets default values
AFGGrid::AFGGrid()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}


// Called when the game starts or when spawned
void AFGGrid::BeginPlay()
{
	Super::BeginPlay();
	MakeGrid();
}

// Called every frame
void AFGGrid::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Timer += DeltaTime;
	FindPath();
	ResetGrid();
}

void AFGGrid::MakeGrid()
{
	NodesList.SetNum(Width * Height);
	for(int y = 0; y < Height; y++)
	{
		for(int x = 0; x < Width; x++)
		{
			FTransform SpawnTransform;
			SpawnTransform.SetLocation(FVector(x * 150,y * 150, 0));
			AFGNode* Node = GetWorld()->SpawnActor<AFGNode>(NodeClass, SpawnTransform);
			Node->XIndex = x;
			Node->YIndex = y;

			Node->FindComponentByClass<UStaticMeshComponent>()->SetMaterial(0, Node->UncheckedM);
			Node->NodeType = NodeType::UNCHECKED;
			
			if(x == 3 && y > -1 && y < 5)
			{
				Node->FindComponentByClass<UStaticMeshComponent>()->SetMaterial(0, Node->BlockedM);
				Node->NodeType = NodeType::BLOCKED;
			}
			NodesList[y * Width + x] = Node;
		}
	}
}

void AFGGrid::ResetGrid()
{
	if(IsReset && Timer > 5)
	{
		for(int y = 0; y < Height; y++)
		{
			for(int x = 0; x < Width; x++)
			{
				AFGNode* Node = NodesList[y * Width + x];
				if(Node->NodeType != NodeType::BLOCKED)
				{
					Node->FindComponentByClass<UStaticMeshComponent>()->SetMaterial(0, Node->UncheckedM);
					Node->NodeType = NodeType::UNCHECKED;
				}
			}
		}
		IsReset = false;
	}
}


void AFGGrid::GetNeighbours(int X, int Y)
{
	TArray<FVector> AllDirections;
	AllDirections.Add(FVector(0,1,0));
	AllDirections.Add(FVector(1,0,0));
	AllDirections.Add(FVector(0,-1,0));
	AllDirections.Add(FVector(-1,0,0));

	NodesList[Y * Width + X]->NeighbourNodes.SetNum(4);
	for(int i = 0; i < AllDirections.Num(); i++)
	{
		int NewX = X + AllDirections[i].X;
		int NewY = Y + AllDirections[i].Y;

		bool IsWithinBounds = NewX >= 0 && NewX < Width && NewY >= 0 && NewY < Height;
		
		if(IsWithinBounds && NodesList[NewY * Width + NewX] != nullptr && NodesList[NewY * Width + NewX]->NodeType != NodeType::BLOCKED && NodesList[NewY * Width + NewX]->NodeType != NodeType::CLOSED)
		{
			NodesList[Y * Width + X]->NeighbourNodes[i] = NodesList[NewY * Width + NewX];
		}
	}
}

void AFGGrid::FindPath()
{
	if(GoalNodeSelected)
	{
		while (CurrentNode->isGoal != true && HasOpenNodes())
		{
		
			GetNeighbours(CurrentNode->XIndex, CurrentNode->YIndex);
			int CurrentFScore = TNumericLimits<int32>::Max();
			AFGNode* NextCurrentNode = nullptr;
			for(int i = 0; i<CurrentNode->NeighbourNodes.Num(); i++)
			{
				if(CurrentNode->NeighbourNodes[i] != nullptr && CurrentNode->NeighbourNodes[i]->NodeType != NodeType::CLOSED)
				{
				
					AFGNode* CurrentNeighbour = CurrentNode->NeighbourNodes[i];
					if(CurrentNeighbour->NodeType != NodeType::OPEN)
					{
						CurrentNeighbour->HScore = GetHScore(CurrentNeighbour->XIndex, CurrentNeighbour->YIndex);
						CurrentNeighbour->GScore = 1 + CurrentNode->GScore;
						CurrentNeighbour->FScore = CurrentNeighbour->GScore + CurrentNeighbour->HScore;
						CurrentNeighbour->NodeType = NodeType::OPEN;
						//CurrentNeighbour->FindComponentByClass<UStaticMeshComponent>()->SetMaterial(0, CurrentNeighbour->OpenedM);
						CurrentNeighbour->Parent = CurrentNode;

					}
					else if(CurrentNeighbour->NodeType == NodeType::OPEN && CurrentNeighbour->Parent != nullptr && CurrentNeighbour->Parent->GScore > CurrentNode->GScore)
					{
						CurrentNeighbour->HScore = GetHScore(CurrentNeighbour->XIndex, CurrentNeighbour->YIndex);
						CurrentNeighbour->GScore = 1 + CurrentNode->GScore;
						CurrentNeighbour->FScore = CurrentNeighbour->GScore + CurrentNeighbour->HScore;
						CurrentNeighbour->Parent = CurrentNode;
					}
				
				
				}
			}

			for(int i = 0; i < NodesList.Num(); i++)
			{
				if(NodesList[i]->NodeType == NodeType::OPEN)
				{
					if(CurrentFScore > NodesList[i]->FScore)
					{
						CurrentFScore = NodesList[i]->FScore;
						NextCurrentNode = NodesList[i];
					}
				}
			}
			CurrentNode->NodeType = NodeType::CLOSED;
			CurrentNode->FindComponentByClass<UStaticMeshComponent>()->SetMaterial(0, CurrentNode->ClosedM);
			CurrentNode = NextCurrentNode;
		}
	
		AFGNode* CurrentFinishedNode = CurrentNode;
		while(CurrentFinishedNode->Parent != nullptr && CurrentNode->isGoal == true)
		{
			CurrentFinishedNode->FindComponentByClass<UStaticMeshComponent>()->SetMaterial(0, CurrentFinishedNode->CorrectM);
			CurrentFinishedNode = CurrentFinishedNode->Parent;
		}

		GoalNodeSelected = false;
		StartNodeSelected = false;
		Timer = 0;
		IsReset = true;
	}
}

int AFGGrid::GetHScore(int X, int Y)
{
	return FMath::Abs(X - GoalNode->XIndex) + FMath::Abs(Y - GoalNode->YIndex);
}

bool AFGGrid::HasOpenNodes()
{
	for(int i = 0; i<NodesList.Num(); i++)
	{
		if(NodesList[i]->NodeType == NodeType::OPEN)
			return true;
	}
	return false;
}



