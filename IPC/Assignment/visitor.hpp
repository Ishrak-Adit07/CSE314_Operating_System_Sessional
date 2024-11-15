#include <iostream>

class Visitor
{
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

class MuseumParameters
{
private:
    int standard_visitors, premium_visitors;
    int hallway_time, gallery1_time, gallery2_time, photo_booth_time;

public:
    MuseumParameters(int standard, int premium, int hallway, int gallery1, int gallery2, int photo_booth)
    {
        standard_visitors = standard; premium_visitors = premium;
        hallway_time = hallway; gallery1_time = gallery1;
        gallery2_time = gallery2; photo_booth_time = photo_booth;
    }

    int getNumberOfStandardVisitors() const { return standard_visitors; }
    void setNumberOfStandardVisitors(int standard) { standard_visitors = standard; }

    int getNumberOfPremiumVisitors() const { return premium_visitors; }
    void setNumberOfPremiumVisitors(int premium) { premium_visitors = premium; }

    int getHallwayTime() const { return hallway_time; }
    void setHallwayTime(int hallway) { hallway_time = hallway; }

    int getGallery1Time() const { return gallery1_time; }
    void setGallery1Time(int gallery1) { gallery1_time = gallery1; }

    int getGallery2Time() const { return gallery2_time; }
    void setGallery2Time(int gallery2) { gallery2_time = gallery2; }

    int getPhotoBoothTime() const { return photo_booth_time; }
    void setPhotoBoothTime(int photo_booth) { photo_booth_time = photo_booth; }

    void setParameters(int standard, int premium, int hallway, int gallery1, int gallery2, int photo_booth)
    {
        standard_visitors = standard; premium_visitors = premium;
        hallway_time = hallway; gallery1_time = gallery1;
        gallery2_time = gallery2; photo_booth_time = photo_booth;
    }
};