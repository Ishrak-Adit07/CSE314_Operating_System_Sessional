#include <iostream>

class Visitor {
private:
    int id;
    int status; // 0 for standard, 1 for premium

public:
    Visitor(int id, int status) : id(id), status(status) {}

    int getId() const { return id; }
    void setId(int new_id) { id = new_id; }

    int getStatus() const { return status; }
    void setStatus(int new_status) { status = new_status; }
};
