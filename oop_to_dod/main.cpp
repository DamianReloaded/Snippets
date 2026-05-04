//---------------------------------------------------
// The public-facing API
class Entity {
public:
    Entity(int id);

    // Behaviors exposed to team
    Vector3 GetPosition() const;
    void SetPosition(const Vector3& pos);
    void ApplyForce(const Vector3& force);

private:
    struct Impl; // hidden implementation
    std::unique_ptr<Impl> pImpl;
};

//---------------------------------------------------
// OOP-style backend (classic)
struct Entity::Impl {
    Vector3 position;
    Vector3 velocity;
    void ApplyForce(const Vector3& force) {
        velocity += force; // simple physics
    }
};

Entity::Entity(int id)
    : pImpl(std::make_unique<Impl>()) {}

Vector3 Entity::GetPosition() const { return pImpl->position; }
void Entity::SetPosition(const Vector3& pos) { pImpl->position = pos; }
void Entity::ApplyForce(const Vector3& force) { pImpl->ApplyForce(force); }

//---------------------------------------------------
// DoD-style backend (cache-friendly)
struct Entity::Impl {
    // Contiguous arrays (Struct of Arrays)
    static std::vector<Vector3> positions;
    static std::vector<Vector3> velocities;
    int index; // position in arrays

    void ApplyForce(const Vector3& force) {
        velocities[index] += force;
    }
};

// Implementation of Entity operations delegates to arrays
Vector3 Entity::GetPosition() const { return Impl::positions[pImpl->index]; }
void Entity::SetPosition(const Vector3& pos) { Impl::positions[pImpl->index] = pos; }
void Entity::ApplyForce(const Vector3& force) { pImpl->ApplyForce(force); }

//---------------------------------------------------
// Team code never changes
Entity e1(0);
e1.SetPosition({0, 0, 0});
e1.ApplyForce({1, 0, 0});
Vector3 pos = e1.GetPosition();


//---------------------------------------------------
// Optional: Batch-Friendly API
class PhysicsSystem {
public:
    static void IntegrateAll(float dt); // operates on all entities in arrays
};