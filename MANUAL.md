# Usage

## Building

Dependencies (on Debian Wheezy/Ubuntu):

    sudo apt-get install libpoco-dev libpocofoundation9 libpoconet9 libpocoutil9 libgdal1 libgdal1-dev cmake g++ build-essential libpq-dev discount python

The software can be build by the following commands:

    cmake .
    make

## Command line arguments

TODO: copy&paste output of help

## Configuration file

The server is configured via an configuration file which is specified by using the `-c` switch on
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
    
    # A optional description to show in the web interface
    # has to be in one line
    #
    # Optional
    # Default: empty
    description = Countries of africa based on http://www.mapmakerdata.co.uk.s3-website-eu-west-1.amazonaws.com/library/stacks/Africa/index.htm
    
    # The source the contents for tha layer should be pulled from
    # This corresponds with the "datasource_name" parameter the ogrinfo utility uses. So
    # ogrinfo might be used to test the value here for correctness
    #
    # Mandatory
    source = testdata/shapes/Africa.shp
    
    # the name of the layer in the datasource
    # This corresponds with the "layer" paramaeter of the ogrinfo program
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
    # Type: Unquoted String. Attirbute filter as described in the OGR documentation
    #       at http://www.gdal.org/ogr/ogr_sql.html in the section of the
    #       WHERE-clause
    # Default: empty
    # Example: myattribute=3
    filter = 1 = 1



The layer section may be repeated for each layer with an unique name.


## Web interface

The batyr server includes an integrated web interface which allows to pull the configured layers and provides an status page as well as a page listing the state of the current work queue.

The web interface is available on the port configured with the `port` setting in the `HTTP` section of the configuration file. The default port is `9090`.


# HTTP-API

The provided HTTP-API is the same which the integrated web interfaces uses and provides the methods described in this part of the documentation. Data is strictly exchanged in the form of JSON objects and the API requires an `application/json` header if data is POSTed to it.

The basic object the API deals with is called a `job` and posesses the following attributes:

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
* `numDeleted`: Number of features deleted by this job.

Example:

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
        "numPulled": 0
    }



## GET /api/v1/jobs.json

A list of all jobs which are known to the server.

Example:

    {
        "maxAgeDoneJobsSeconds": 600,
        "jobs": []
    }


## GET /api/v1/layers.json

Returns all currently configured layers with their names and description.

Example:

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

Example:

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

Example request:

GET /api/job/1ab8c197ed014a4cbc20a6dfc98a1b101b10.json

Corresponding response when an existing job id was used:

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
        "numPulled": 2
    }

Response when no such job exists:

    {
        "message": "No job with the id 1ab8c197ed014a4cbc20a6dfc98a1b101b10 found"
    }


## POST /api/v1/pull

Allows starting a new job by POSTing a JSON document to this URL. The `layerName` parameter is mandatory while the `filter` parameter is optional. The request will return a job object with the properties of the newly created job. Returns an HTTP status `200` if the request was successful and `400` if the send data was incorrect.

Example Post:

    {
        "layerName":"africa",
        "filter":"id=\"4\""
    }

Corresponding response:

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
        "numPulled": 0
    }


## POST /api/v1/remove-by-attributes

Remove features from the database by matching their columns to attributes of JSON objects. It is possible mutiple criteria in one request

Example Post:

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


Corresponding response:

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
        "numPulled": 0
    }



# Software used

* The [GDAL/OGR](http://gdal.org/) library
* The [libpq](http://www.postgresql.org/docs/current/static/libpq.html) library to connect to PostgreSQL.
* [POCO Framework](http://pocoproject.org/) for its embedded HTTP server as well as a generic application framework.
* [cmake](http://www.cmake.org/) to build the software.
* [AngularJS](http://www.angularjs.org/) as the clientside javascript framework.
* [jQuery](http://jquery.com/) for animations.
* [Moment.js](http://momentjs.com/) date and time formatting.
* [Pure CSS](http://purecss.io/) for its CSS styles.
* [Font Awesome](http://fortawesome.github.io/Font-Awesome/) as iconset.


# Development

see the DEVELOPMENT.md file.


