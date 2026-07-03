#pragma once

/**
 * Model.h
 *
 * Abstract lifecycle interface for every renderable shape owned by Renderer.
 */

class Model {
public:
    Model() {}
    virtual ~Model() {}

    virtual void InitModel() = 0;
    virtual void Render() = 0;
    virtual void Resize(int, int) {}
};
