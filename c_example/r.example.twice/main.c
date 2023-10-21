
/****************************************************************************
 *
 * MODULE:       r.example.twice
 * AUTHORS:      Alice Doe <alice_doe at somewhere org>
 *               Bob Doe <bob_doe at somewhere org>
 * PURPOSE:      Provide short description of module here...
 * COPYRIGHT:    (C) 2022 by Alice Doe and the GRASS Development Team
 *
 *               This program is free software under the GNU General Public
 *               License (>=v2). Read the COPYING file that comes with GRASS
 *               for details.
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <grass/gis.h>
#include <grass/glocale.h>
#include <grass/raster.h>

static double times_two(double a)
{
    return 2 * a;
}

int main(int argc, char *argv[])
{
    // Initializes the GRASS library based on the current GRASS session.
    G_gisinit(argv[0]);

    // Interface
    struct GModule *module = G_define_module();

    G_add_keyword(_("raster"));
    G_add_keyword(_("algebra"));
    G_add_keyword(_("multiplication"));
    module->description = _("Multiply values in a raster map by two");

    struct Option *input = G_define_standard_option(G_OPT_R_INPUT);
    struct Option *output = G_define_standard_option(G_OPT_R_OUTPUT);

    if (G_parser(argc, argv))
        exit(EXIT_FAILURE);

    // Returns a file descriptor for reading a raster or fails.
    int input_fd = Rast_open_old(input->answer, "");

    // Determine data type of the input, we will use it for the output, too.
    // This means we will preserve the input data type on output.
    RASTER_MAP_TYPE data_type = Rast_map_type(input->answer, "");

    // Returns a file descriptor for writing a raster or fails.
    int output_fd = Rast_open_new(output->answer, data_type);

    // Allocate buffer for a row of input and output data.
    // Our computation always happens for doubles, so we will use data
    // converted to doubles. Number of elements is automatically determined
    // from the current computational region.
    DCELL *input_buffer = Rast_allocate_d_buf();
    DCELL *output_buffer = Rast_allocate_d_buf();

    // Get number of rows and columns determined by the current
    // computational region.
    int nrows = Rast_window_rows();
    int ncols = Rast_window_cols();

    // Process each row.
    // Our computation is using only one individual value at a time,
    // so we just iterate over all rows and columns.
    for (int row = 0; row < nrows; row++) {
        G_percent(row, nrows, 10);      // Show only 10% increments.

        // Read a row of input data.
        // This uses computational region, takes into account global mask,
        // and the type of underlying data.
        Rast_get_d_row(input_fd, input_buffer, row);

        // Process each cell in a row.
        for (int col = 0; col < ncols; col++) {
            // The actual computation is called here.
            // This is also the place to deal with null values.
            // For floats and doubles, the code may work even without explicit
            // handling of null values, but the behavior is platform-dependent,
            // so explicit null handling is recommend.
            if (Rast_is_d_null_value(&input_buffer[col]))
                Rast_set_d_null_value(&output_buffer[col], 1);
            else
                output_buffer[col] = times_two(input_buffer[col]);
        }

        // Write a row of output data.
        // Conversion to the output data type is done in the background.
        Rast_put_d_row(output_fd, output_buffer);
    }
    G_percent(1, 1, 1);         // Report 100%.

    // Free buffers and close rasters.
    G_free(input_buffer);
    G_free(output_buffer);
    Rast_close(input_fd);
    Rast_close(output_fd);

    // Add command line parameters to metadata (history).
    // In the background, it uses the parsed command line including
    // default values to construct the history record.
    struct History history;

    Rast_short_history(output->answer, "raster", &history);
    Rast_command_history(&history);
    Rast_write_history(output->answer, &history);

    exit(EXIT_SUCCESS);
}
