// REFERENCES
// https://oceancolor.gsfc.nasa.gov/docs/format/Ocean_Level-3_SMI_Products.pdf

#include <iostream>
#include <string>
#include <fstream>
#include <cstddef>

#include <netcdfcpp.h>

using namespace std;

int main(int argc, char** argv){

    NcFile nc("/home/morrigan/datos.nc");

    cout << "Statistics of datos.nc: \n\n";
    cout << "Is valid? " << nc.is_valid() << endl;
    cout << "Number of dimensions: " << nc.num_dims() << endl;
    cout << "Number of variables: " << nc.num_vars() << endl;
    cout << "Number of global attributes: " << nc.num_atts() << endl << endl;

    // Global Attributes list
    cout << "\n Global attributes:" << endl;
    for(int i = 0; i < nc.num_atts(); i++){
        NcAtt* nca = nc.get_att(i);
        cout << "\t #idx = " << i << "  --  " << string(nca->name());
        cout << "\t = \t" << string(nca->as_string(0)) << endl;
    }

    // Dimensions list
    cout << "\n List of dimensions:" << endl;
    for(int i = 0; i < nc.num_dims(); i++){
        NcDim* dim = nc.get_dim(i);
        cout << "\t #idx = " << i
             << "  --  " << string(dim->name())
             << ", size " << dim->size() << endl;
    }

    // Variables list
    cout << "\n List of variables:" << endl;
    for(int i = 0; i < nc.num_vars(); i++){
        NcVar* ncv = nc.get_var(i);
        cout << "\t #idx = " << i
             << "  --  " << string(ncv->name()) << ", "
             << ncv->num_dims() << " dims, "
             << ncv->num_atts() << " atts, "
             << ncv->num_vals() << " vals, ";
        switch(ncv->type()){
            case NC_NAT: cout << "NC_NAT"; break;
            case NC_BYTE: cout << "NC_BYTE"; break;
            case NC_CHAR: cout << "NC_CHAR"; break;
            case NC_SHORT: cout << "NC_SHORT"; break;
            case NC_INT: cout << "NC_INT"; break;
            //case NC_LONG: cout << "ncLong"; break; // deprecated format
            case NC_FLOAT: cout << "NC_FLOAT"; break;
            case NC_DOUBLE: cout << "NC_DOUBLE"; break;
            case NC_UBYTE: cout << "NC_UBYTE"; break;
            case NC_USHORT: cout << "NC_USHORT"; break;
            case NC_UINT: cout << "NC_UINT"; break;
            case NC_INT64: cout << "NC_INT64"; break;
            case NC_UINT64: cout << "NC_UINT64"; break;
            case NC_STRING: cout << "NC_STRING"; break;
            default: cout << "Unknown type: " << ncv->type(); break;
        }
        cout << endl;

        for(int j = 0; j < ncv->num_atts() > 0; j++){
            cout << "\t\t\t" << ncv->get_att(j)->name() << " ---- " << ncv->get_att(j)->as_string(0) << endl;
        }
    }

    // Print first values for longitude and latitude to verify starting points
    cout << "\n\n\nChecking I/O function" << endl;
    float f_val = 0;
    NcVar* ncv_lon_x = nc.get_var("lon");
    NcVar* ncv_lat_y = nc.get_var("lat");
    unsigned long Nx = ncv_lon_x->num_vals(), Ny = ncv_lat_y->num_vals();

    ncv_lat_y->set_cur(0L);
    ncv_lat_y->get(&f_val, 1L);
    cout << "Latitude start value: " << f_val << endl;
    ncv_lat_y->set_cur(Ny-1);
    ncv_lat_y->get(&f_val, 1L);
    cout << "Latitude end value: " << f_val << endl;

    ncv_lon_x->set_cur(0L);
    ncv_lon_x->get(&f_val, 1L);
    cout << "Longitude start value: " << f_val << endl;
    ncv_lon_x->set_cur(Nx-1);
    ncv_lon_x->get(&f_val, 1L);
    cout << "Longitude end value: " << f_val << endl;

    cout << "\n\nObservation: latitude start value NOT EQUALS to sw_point_latitude attribute!" << endl;
    cout << "Desision: using matrix values." << endl;


    //****************************************************************************************



    // General gnuplot configuration
    ofstream plot("/home/morrigan/Convocatorias_2018/SienciasDelMar/TecnicoComputo/examen/autoplot.plt");


    plot <<
R"(
set terminal png size 8640, 4320
set output 'out.png'
#set terminal X11
set title 'MODIS sattelite: Sea Surface Temperature (NetCDF examen)'

set palette file '-'
)";

    // Get Palette from NetCDF "palette" variable
    auto var = nc.get_var("palette");
    int * cc = new int[3*256];
    var->get(cc, 3L, 256L);

    for(int idx = 0; idx < 256; idx++) {
        for (int rgb = 0; rgb < 3; rgb++) {
            plot << (float)cc[idx*3 + rgb]/255.0 << (rgb < 2 ? " " : "");
        }
        plot << (idx < 255 ? "\n" : "\ne\n");
    }
    delete [] cc;


    // Setup Pm3D in gnuplot script
    plot <<
R"(
set view map scale 1
set colorbox vertical
set style data pm3d
set style function pm3d
#set pm3d implicit at b
#set xyplane relative 0
#unset surface
show colorbox
set cbrange [ -2 : 45 ] noreverse nowriteback
)";

    // Getting data
    NcVar* ncv_sst = nc.get_var("sst");

    // Each scanline goes over different latitudes, I won't calculate them on the fly, so
    float sst_min = nc.get_att("data_minimum")->as_float(0L);
    float sst_max = nc.get_att("data_maximum")->as_float(0L);

    // Scanlines for gnuplot shell have same X-coord, so it's dimension is latitude dependent
    short* sst_scanline = new short[Ny];
    float* lat_y = new float[Ny];

    plot << "splot '-'" << endl;

    // Get all latitudes
    ncv_lat_y->set_cur(0L);
    ncv_lat_y->get(lat_y, Ny-1);

    for(long i = 0; i < Nx; i += 2){
        // get current scanline longitude
        ncv_lon_x->set_cur(i);
        ncv_lon_x->get(&f_val, 1L);

        // set sst column and get it's data
        ncv_sst->set_cur(0L, i);
        ncv_sst->get(sst_scanline, Ny, 1L);

        // sent data to the plot file
        for(long j = 0; j < Ny; j += 2){
            //auto curr_sst = ((float)sst_scanline[j]/65535.0)*(sst_max - sst_min) + sst_min;
            auto curr_sst = (float)sst_scanline[j]*0.005;
            if(sst_scanline[j] == -32767) curr_sst = 45;
            plot << f_val << " " << lat_y[j] << " " << curr_sst << endl;
        }
        plot << endl;
    }
    plot << "e\n";

    delete [] sst_scanline;
    delete [] lat_y;



    return 0;
}

