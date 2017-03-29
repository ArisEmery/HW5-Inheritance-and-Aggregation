//
// Created by Stephen Clyde on 3/4/17.
//

#include "Region.h"
#include "Utils.h"
#include "World.h"
#include "Nation.h"
#include "State.h"
#include "County.h"
#include "City.h"
#include <iostream>
#include <iomanip>

const std::string regionDelimiter = "^^^";
const int TAB_SIZE = 4;
unsigned int Region::m_nextId = 0;
Region *rootRegion;

Region* Region::create(std::istream &in)
{
    Region* region = nullptr;
    std::string line;
    std::getline(in, line);
    if (line!="")
    {
        region = create(line);
        if (region!= nullptr)
            region->loadChildren(in);
    }
    return region;
}
Region* Region::create(const std::string& data)
{
    Region* region = nullptr;
    std::string regionData;
    unsigned long commaPos = (int) data.find(",");
    if (commaPos != std::string::npos)
    {
        std::string regionTypeStr = data.substr(0,commaPos);
        regionData = data.substr(commaPos+1);

        bool isValid;
        if(regionTypeStr=="2"){ return nullptr;};
        RegionType regionType = (RegionType) convertStringToInt(regionTypeStr, &isValid);
       // if(regionTypeStr=="Arisland"){
         //   isValid=true;
        //}
        if (isValid)
        {
            region = create(regionType, regionData);
        }

    }

    return region;
}

Region* Region::create(RegionType regionType, const std::string& data)
{
    Region* region = nullptr;
    std::string fields[3];
    if (split(data, ',', fields, 3)) {

        // Create the region based on type
        switch (regionType) {
            case WorldType:
                region = new World();
                rootRegion=region;
                break;
            case NationType:
                region = new Nation(fields);
                break;
            case StateType:
                region = new State(fields);
                break;
            case CountyType:
                region = new County(fields);
                break;
            case CityType:
                region = new City(fields);
                break;
            default:
                break;
        }

        // If the region isn't valid, git ride of it
        if (region != nullptr && !region->getIsValid()) {
            delete region;
            region = nullptr;
        }
    }
    region->regionData=data;
    region->subRegions.clear();
    return region;
}

std::string Region::regionLabel(RegionType regionType)
{
    std::string label = "Unknown";
    switch (regionType)
    {
        case Region::WorldType:
            label = "World";
            break;
        case Region::NationType:
            label = "Nation";
            break;
        case Region::StateType:
            label = "State";
            break;
        case Region::CountyType:
            label = "County";
            break;
        case Region::CityType:
            label = "City";
            break;
        default:
            break;
    }
    return label;
}

Region::Region() { }

Region::Region(RegionType type, const std::string data[]) :
        m_id(getNextId()), m_regionType(type), m_isValid(true)
{
    m_name = data[0];
    m_population = convertStringToUnsignedInt(data[1], &m_isValid);
    if (m_isValid)
        m_area = convertStringToDouble(data[2], &m_isValid);
}

Region::~Region()
{
    subRegions.clear();
}

std::string Region::getRegionLabel() const
{
    return regionLabel(getType());
}

unsigned int Region::computeTotalPopulation(Region region)
{
    int subRegionPop=0;
    if(!region.subRegions.empty()){

        for(int i=0;i<region.subRegions.size();i++){
            subRegionPop+=computeTotalPopulation(region.subRegions[i]);
        }
    }
    subRegionPop+=region.m_population;
    return subRegionPop;
}

void Region::list(std::ostream& out)
{
    out << std::endl;
    out << getName() << ":" << std::endl;
    for(int i=0;i<subRegions.size();i++){
        //out<<getName();
        out<<"ID: "<<subRegions[i].getId()<<" - ";
        out<<"Name: "<<subRegions[i].getName()<<std::endl;
    }

    // foreach subregion, print out
    //      id    name
}

void Region::display(std::ostream& out, unsigned int displayLevel, bool showChild, Region region)
{
    if (displayLevel>0)
    {
        out << std::setw(displayLevel * TAB_SIZE) << " ";
    }
    Region region2=region;
    unsigned totalPopulation = region.computeTotalPopulation(region);
    double area = region.getArea();
    double density = (double) totalPopulation / area;
    std::string string1=region.getName();

    out << std::setw(6) << region.getId() << "  "
        << region.getName() << ", population="
        << totalPopulation
        << ", area=" << area
        << ", density=" << density << std::endl;

    if (showChild)
    {

        // foreach subregion
        //      display that subregion at displayLevel+1 with the same showChild value
        for(int i=0;(i<region.subRegions.size());i++){
                display(std::cout, (displayLevel + 1), true, region.subRegions[i]);


        }
    }
}

void Region::save(std::ostream& out, Region region)
{
    out << region.getType()
        << "," << region.getName()
        << "," << region.getPopulation()
        << "," << region.getArea()
        << std::endl;
    for(int i=0;region.subRegions.size();i++) {
        out << region.subRegions[i].getType()
            << "," << region.subRegions[i].getName()
            << "," << region.subRegions[i].getPopulation()
            << "," << region.subRegions[i].getArea()
            << std::endl;
    }
    out << regionDelimiter << std::endl;
}

void Region::validate()
{
    m_isValid = (m_area!=UnknownRegionType && m_name!="" && m_area>=0);
}

void Region::loadChildren(std::istream& in)
{
    std::string line;
    bool done = false;
    while (!in.eof() && !done)
    {
        std::getline(in, line);
        if (line==regionDelimiter)
        {
            done = true;
        }
        else
        {
            Region* child = create(line);
            if (child!= nullptr)
            {
                subRegions.push_back(*child);
                child->loadChildren(in);
            }
        }
    }
}

unsigned int Region::getNextId()
{
    if (m_nextId==UINT32_MAX)
        m_nextId=1;

    return m_nextId++;
}

void Region::add(Region region) {
    subRegions.push_back(region);
}

std::string Region::getData() {
    return regionData;
}

void Region::setId(int x){
    m_id=x;
}
void Region::addSubRegion(Region region) {
    subRegions.push_back(region);
}

