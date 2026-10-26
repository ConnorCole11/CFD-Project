#include <array>




class RectangularChannel {
public:

    RectangularChannel(
        double length, 
        double width, 
        double height, 
        std::array<double, 3> center 
    );

    double length() const;
    double width() const;
    double height() const;
    std::array<double, 3> center() const;

private:
    double const length_;
    double const width_;
    double const height_;
    std::array<double, 3> center_;

};