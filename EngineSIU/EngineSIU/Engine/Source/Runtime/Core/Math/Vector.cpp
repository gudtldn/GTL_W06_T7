#include "Vector.h"

const FVector2D FVector2D::ZeroVector = FVector2D(0, 0);
const FVector2D FVector2D::OneVector = FVector2D(1, 1);

const FVector FVector::ZeroVector = FVector(0, 0, 0);
const FVector FVector::OneVector = FVector(1, 1, 1);

const FVector FVector::UpVector = FVector(0, 0, 1);
const FVector FVector::DownVector = FVector(0, 0, -1);
const FVector FVector::ForwardVector = FVector(1, 0, 0);
const FVector FVector::BackwardVector = FVector(-1, 0, 0);
const FVector FVector::RightVector = FVector(0, 1, 0);
const FVector FVector::LeftVector = FVector(0, -1, 0);

const FVector FVector::XAxisVector = FVector(1, 0, 0);
const FVector FVector::YAxisVector = FVector(0, 1, 0);
const FVector FVector::ZAxisVector = FVector(0, 0, 1);
