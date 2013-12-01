# batyr Manual

This application is a server for on-demand synchronization of vector datasources to a PostgreSQL/PostGIS database.
It offers JSON-based HTTP REST webservice which may be called by external applications to trigger the synchronization of geodata
into a PostgreSQL database. 

The name "batyr" originates from an Asian elephant who lived in a zoo in Kazakhstan. This elephant was claimed of being able to
speak with a vocabulary of about 20 words. This ability and the fact that the official logo of the PostgreSQL also being an elephant
made the name of the animal somewhat fitting for an application which main purpose is communicating with remote datasources to read
their data into a PostgreSQL database. For more information on this elephant refer to the corresponding 
[Wikipedia article](https://en.wikipedia.org/wiki/Batyr).


## The synchronization process

The synchronization process can be divided into six steps:

1. batyr creates a new temporary table in the database which uses the same schema definition as the target table.
2. data is pulled from the source and gets written to the new temporary table.
3. batyr uses the primary key definition of the target table to update the contents of the target table using the newly fetched contents of the temporary table. The update will only affect rows where data actually differ to reduce the number of writes and the amount of possibly defined triggers firing.
4. batyr checks the temporary table for rows which are missing in the target table using the primary key and inserts these into the target table.
5. batyr deletes all rows from the target table which are not part of the new data. This step is optional and may be disabled by the `allow_feature_deletion` setting and also is generally deactivated when a filter is used.
6. The temporary table gets dropped again.

These six steps are performed inside a transaction and will all get rolled back in case of an error.

The handling of different coordinate systems relies of the PostGIS geometry_columns view or - in older versions - table. batyr will use the SRID information from there to transform incoming geometries to the coordinate system of the target table. Incoming data without coordinate system information will get this SRID assigned without an transformation.

This synchronization itself is performed asynchronously. This means that after an external application sends a request to pull a layer,
the data in the database might not yet be updated when the request is finished. Instead the request gets queued internally and
will be worked upon in the background. The main purpose of this behavior is to avoid blocking the requesting application in case
there is a lot of data to fetch or there are many other requests batyr also has to handle.


# Usage

## Building

In case you build batyr from sources, the following dependencies (Debian Wheezy/Ubuntu packages) are required:

    sudo apt-get install libpoco-dev libpocofoundation9 libpoconet9 libpocoutil9 libgdal1 libgdal1-dev cmake g++ build-essential libpq-dev discount python

The software can be built by the following commands:

    cmake .
    make

## Command line arguments

    usage: batyrd -c=CONFIGFILE [OPTIONS]
    Server for on-demand synchronization of vector datasources to 
    a PostgreSQL/PostGIS database.

    version: 0.1.2 [git: 5e8d43ce50]

    --daemon                   run application as a daemon
    --pidfile=path             write PID to given file
    -h, --help                 Display help information and exit.
    -cfile, --configfile=file  Path to the configuration file.


## Web interface

The batyr server includes an integrated web interface which allows to pull the configured layers and provides a status page as well as a page listing the state of the current job queue.

The web interface is available on the port configured with the `port` setting in the `HTTP` section of the configuration file. The default port is `9090`.


## Configuration file

The server is configured via a configuration file which is specified by using the `-c` switch on
the command line.

    batyrd -c=/etc/my-batyr-config-file.cfg

The valid values for each setting are documented in the example file bellow. For boolean values the following values are valid

* True: yes, true, y, t, 1
* False: no, false, n, f, 0

### Example configuration file

    # Configuration for the batyr server
    #
    # Syntax hints:
    #    - valid values for boolean settings are yes,no,true,false,1,0
    #    - multiline values are possible by starting the line after the key with a "+"
    
    
    [MAIN]
    
    # The number of worker threads to launch
    #
    # Optional
    # Type: integer; must be > 1
    # Default: 2
    num_worker_threads = 4
    
    
    # The time after which finished and failed jobs are removed
    # As all jobs are kept in memory this time should not be set too
    # high.
    #
    # The units are seconds
    #
    # Optional
    # Type: integer; must be > 1; 
    # Default: 600
    max_age_done_jobs = 600
    
    
    # Connection to the postgresql database
    # A connection string containing all parameters needed to connect
    # to the database. The syntax is described in the postgresql manual
    # on
    # http://www.postgresql.org/docs/9.2/static/libpq-connect.html#LIBPQ-PARAMKEYWORDS
    #
    # Mandatory
    dsn = "dbname=batyr user=batyr password=batyr host=localhost"
    

    # keep database connections open even when there are currently
    # no jobs to handle
    #
    # Optional
    # Type: boolean
    # Default: yes
    use_persistent_connections = yes
    
    
    # Logging settings
    [LOGGING]
    
    # The loglevel of the application
    #
    # valid values are: "error", "warning", "information" and
    # "debug", although "debug" may only be used with DEBUG
    # builds
    #
    # Optional
    # Default: "information"
    loglevel = debug
    
    # logfile
    # If this setting is not set or empty, all log messages will be written to stdout
    #
    # Optional
    # Default: <not set>
    logfile = 
    
    
    # Settings for the HTTP-Interface
    [  HTTP  ]
    
    # The port to listen on
    #
    # Optional
    # Default: 9090
    port = 9091
    
    
    # the layer configurations
    [LAYERS]
    
    # Example layer. The name in the "[[" "]]" will be used as the unique
    # indentifier for the layer.
    [[africa]]
    
    # enable/disable this layer. Each layer is enabled by default
    #
    # Optional
    # Default: yes
    enabled = true
    
    # A optional description to show in the web interface
    # has to be in one line
    #
    # Optional
    # Default: empty
    description = Countries of africa based on http://www.mapmakerdata.co.uk.s3-website-eu-west-1.amazonaws.com/library/stacks/Africa/index.htm
    
    # The source the contents for the layer should be pulled from
    # This corresponds with the "datasource_name" parameter the ogrinfo utility uses. So
    # ogrinfo might be used to test the value here for correctness
    #
    # Mandatory
    source = testdata/shapes/Africa.shp
    
    # the name of the layer in the datasource
    # This corresponds with the "layer" parameter of the ogrinfo program
    #
    # Mandatory
    source_layer = Africa
    
    # The name of table the layers contents should be written to
    # The schema has to be omitted here.
    #
    # Mandatory
    target_table_name = africa
    
    # The name of schema of the table referenced in target_table_name
    #
    # Mandatory
    target_table_schema = test
    
    # Allow features to be deleted during the pull.
    # Features which are not part of the source will be removed from the target.
    # This setting is always disabled when a filter is used
    #
    # Optional
    # Type: boolean
    # Default: no
    allow_feature_deletion = yes
    
    # Filter the features of the source by a criteria to pull only matching
    # features in the db.
    #
    # The features might be further filtered by the "filter" parameter
    # of a pull request
    #
    # Optional
    # Type: Unquoted String. Attribute filter as described in the OGR documentation
    #       at http://www.gdal.org/ogr/ogr_sql.html in the section of the
    #       WHERE-clause
    # Default: empty
    # Example: myattribute=3
    filter = 1 = 1
    
    # Ignore features with attributes which can not be cast to the datatype
    # of the table. The default behaviour is failing the whole pull when one
    # single feature can not be casted.
    #
    # Optional
    # Type: boolean
    # Default: no
    ignore_failures = no
    
    # Override/set the primary key for the table of this layer.
    #
    # Optional. The normal behaviour would be analyzing the schema of the table to find
    #           the column(s) of the primary key
    # Type: comma-seperated list of column names
    # Default: empty
    primary_key_columns = id



The layer section may be repeated for each layer with a unique name.

## Setting up the database tables

batyr will not create tables nor make any modifications of the database schema by itself. So the user has to create the table herself/himself.

In general the mapping of fields in the source data to the columns of the target table is based on the names of the fields. This means an attribute of an incoming feature will be written to the column with the corresponding name. If no such column exists, the attribute will simply be ignored. Columns of the target table which are also not part of the source features will also be left untouched. batyr will attempt to cast values to the datatype of the column table. This approach will work for most cases, but has a few limitations: Writing an attribute of the type text to a column in the target table of the type integer will work as long as the source only contains numeric values. When the source attribute may also contain letters or other non-numeric characters the sync will fail. This is something to keep in mind when designing the schema of the target table.

The target table is required to have a primary key which is also part of the source data - this is quite important as the values of the primary key are the basis of the synchronization.

The geometry column of the table should have a constraint which defines the spatial reference system of the column to allow batyr to reproject incoming geometries to the correct coordinate system. Earlier PostGIS versions require this information to be also stored in the `geometry_columns` table, which may be automatically added using the [`Populate_Geometry_Columns`](http://postgis.org/docs/Populate_Geometry_Columns.html) function.

Columns may be added or removed from the target table without restarting batyr.

Current limitations are:

* Only tables with one geometry column are supported
* Views are not supported as there is no primary key information available. This issue is already on the roadmap to be resolved in the future


# HTTP-API

The provided HTTP-API is the same which the integrated web interfaces uses and provides the methods described in this part of the documentation. Data is strictly exchanged in the form of JSON objects and the API requires an `application/json` header if data is POSTed to it.

The basic object the API deals with is called a `job` and possesses the following attributes:

* `id`: Identifier of the job. This value is always present.
* `type`: Type of job. This value is always present as possible values are: `pull` and `remove-by-attributes`.
* `timeAdded`: Timestamp when the job was received. Always present.
* `status`: Status of the job. Possible values are `queued`, `in_progress`, `finished` and `failed`. Always present.
* `layerName`: Name of the layer the job wants to pull.  Always present.
* `filter`: Attribute filter. Optional. Only used with pull-jobs.
* `message`: A message from the server regarding this job. Mostly empty, but will contain an error message in case something went wrong.
* `numPulled`: Number of features pulled/read from the source.
* `numCreated`: Number of newly created features in the database.
* `numUpdated`: Number of existing features in the database which have been updated. Features will only be updated if they show differences.
* `numIgnored`: Number of features ignored because of one or more of their attributes havig an type incompatible with the table in the database. This beviour has to be enabled in the configfile.
* `numDeleted`: Number of features deleted by this job.

### Example

    {
        "id": "c94a6c77c18649668fd780744ea745a645a6",
        "type": "pull",
        "timeAdded": "2013-10-08T13:58:51Z",
        "status": "queued",
        "layerName": "africa",
        "filter": "id=\"4\"",
        "message": "",
        "numCreated": 0,
        "numUpdated": 0,
        "numDeleted": 0,
        "numIgnored": 0,
        "numPulled": 0
    }



## GET /api/v1/jobs.json

A list of all jobs which are known to the server.

### Example

    {
        "maxAgeDoneJobsSeconds": 600,
        "jobs": []
    }


## GET /api/v1/layers.json

Returns all currently configured layers with their names and description.

### Example

    {
        "layers": [
            {
                "name": "africa",
                "description": "Countries of africa based on http://www.mapmakerdata.co.uk.s3-website-eu-west-1.amazonaws.com/library/stacks/Africa/index.htm"
            },
            {
                "name": "dataset1",
                "description": "testing different values"
            }
        ]
    }


## GET /api/v1/status.json

This method returns an object with the configuration and the current state of the server. The key `numFailedJobs` might be used to monitor the server using a tool like for example nagios.

### Example

    {
        "appName": "batyrd",
        "appVersion": "0.1.0",
        "appGitVersion": "96835a2f99",
        "numLayers": 2,
        "numQueuedJobs": 0,
        "numFinishedJobs": 0,
        "numFailedJobs": 0,
        "numInProcessJobs": 0,
        "numWorkers": 4
    }


## GET /api/v1/job/[job id].json

Fetch a job object by its id.

### Example request

GET /api/job/1ab8c197ed014a4cbc20a6dfc98a1b101b10.json

### Corresponding response when an existing job id was used

    {
        "id": "1ab8c197ed014a4cbc20a6dfc98a1b101b10",
        "type": "pull",
        "timeAdded": "2013-10-10T07:23:49Z",
        "timeFinished": "2013-10-10T07:23:49Z",
        "status": "finished",
        "layerName": "dataset1",
        "filter": "",
        "message": "",
        "numCreated": 0,
        "numUpdated": 2,
        "numDeleted": 0,
        "numIgnored": 0,
        "numPulled": 2
    }

### Response when no such job exists

    {
        "message": "No job with the id 1ab8c197ed014a4cbc20a6dfc98a1b101b10 found"
    }


## POST /api/v1/pull

Allows starting a new job by POSTing a JSON document to this URL. The `layerName` parameter is mandatory while the `filter` parameter is optional. The request will return a job object with the properties of the newly created job. Returns an HTTP status `200` if the request was successful and `400` if the sent data was incorrect.

### Example POST

    {
        "layerName":"africa",
        "filter":"id=\"4\""
    }

### Corresponding response

    {
        "id": "c94a6c77c18649668fd780744ea745a645a6",
        "type": "pull",
        "timeAdded": "2013-10-08T13:58:51Z",
        "status": "queued",
        "layerName": "africa",
        "filter": "id=\"4\"",
        "message": "",
        "numCreated": 0,
        "numUpdated": 0,
        "numDeleted": 0,
        "numIgnored": 0,
        "numPulled": 0
    }


## POST /api/v1/remove-by-attributes

Remove features from the database by matching their columns to attributes of JSON objects. It is possible to specify multiple criteria in one request

This request is more or less an additional feature for applications which need to selectively remove features from the database. In general performing a full sync using the `pull` API method is the preferred way of ensuring consistent data.

### Example POST

    {
        "layerName":"africa"
        "attributeSets": [
            {
                "column1": "some value",
                "column2": null
            },
            {
                "column3": "some other value"
            }
        ]
    }

This request will remove all features where the column1 is equal to "some value" and column2 is null as well
as all features where column3 equals "some other value". Values for attribute have to be strings or `null`.

### Corresponding response

    {
        "id": "c94a6c77c18649668fd780744ea745a645a6",
        "type": "remove-by-attributes",
        "timeAdded": "2013-10-08T13:58:51Z",
        "status": "queued",
        "layerName": "africa",
        "attributeSets": [
            {
                "column1": "some value",
                "column2": null
            },
            {
                "column3": "some other value"
            }
        ],
        "message": "",
        "numCreated": 0,
        "numUpdated": 0,
        "numDeleted": 0,
        "numIgnored": 0,
        "numPulled": 0
    }



# Software used

* The [GDAL/OGR](http://gdal.org/) library
* The [libpq](http://www.postgresql.org/docs/current/static/libpq.html) library to connect to PostgreSQL.
* [POCO Framework](http://pocoproject.org/) for its embedded HTTP server as well as a generic application framework.
* [rapidjson](https://code.google.com/p/rapidjson/) for parsing and generating JSON.
* [cmake](http://www.cmake.org/) to build the software.
* [AngularJS](http://www.angularjs.org/) as the clientside javascript framework.
* [jQuery](http://jquery.com/) for animations.
* [Moment.js](http://momentjs.com/) date and time formatting.
* [Pure CSS](http://purecss.io/) for its CSS styles.
* [Font Awesome](http://fortawesome.github.io/Font-Awesome/) as iconset.


# Development

The development of batyr has been sponsored by the [trafimage](http://www.trafimage.ch) project of Swiss Federal Railways [SBB](http://www.sbb.ch).

For development related notes see the included DEVELOPMENT.md file.


# License

Copyright (c) 2013, geOps

All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of the geOps nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
