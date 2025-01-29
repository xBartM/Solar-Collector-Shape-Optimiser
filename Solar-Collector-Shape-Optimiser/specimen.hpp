#ifndef SPECIMEN_HPP
#define SPECIMEN_HPP


class Specimen { // RENAME TO Genes
public:
    uint32_t id;   // unique identifier for a Specimen (form 0 to a-lot)

    uint32_t chromosome_size; // number of chromosomes
    double* shape;  // table for heights at x and y coordinates -- access via (getter, but..) [y*xsize+x] -- this is chromosome
    double fitness; // fitness of a specimen (% of triangles that reflected the light)


    Specimen(const uint32_t id, const uint32_t chromosome_size);
    Specimen (const Specimen & other);
    Specimen& operator= (const Specimen & other);
    ~Specimen();

    // double getXY(const unsigned short x, const unsigned short y) const;
    // void setXY(const unsigned short x, const unsigned short y, const double val);
    // void showYourself();
    
};


#endif // SPECIMEN_HPP