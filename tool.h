#pragma once

class tool // Abstract base class allowing all position types in the different EngineX namespaces to be grouped under the same type.
{
public:
    virtual ~tool() {}

    // This class only has pure virtual methods in the Versus Sim. In the main engine project (this project), a tool
    // object/pointer is never instantiated since there's only ONE engine playing against the user!
};
