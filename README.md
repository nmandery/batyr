# batyr

A server which connects all kinds of vector geodata sources to a PostgreSQL/PostGIS database and provides a structured way to synchronize external data to database tables.

One common situation when dealing with geographic data is repeatedly exporting and importing this data to and from a PostGIS-enabled database. While the export is very well covered by products like Mapserver and GeoServer, importing is a bit more tricky. Common solutions consist mostly of custom scripts wrapping commands like shp2pgsql or ogr2ogr. These solutions often fail or at least need some tricky hacks if single rows of data should be updated instead of deleting and restoring the complete table content. It is also hard to account for slow or interrupted transactions and still make sure that the data stays synchronized as a whole. Using these import scripts requires either command line access or some custom code to hook them up to a job queue or even web interface to make them usable from within other applications.

Flaws like those were the reason for us to create batyr as a reusable solution for similar demands in the future.

![Screenshot of the status page of the webinterface](images/statuspage_520b_1.png?raw=true)

batyr is a single server application providing the following:

* "Intelligent" writing of data. A synchronization does not consist of a complete truncate and restore of a table anymore. Only features which have any differences to the ones provided by the external datasource are actually updated. New features are only created if they are not already in the database and features get (optionally) removed from the database if they are not part of the datasource any more. All this uses the primary key of the table to identify matching features from the datasource.
* An integrated web-interface to get an overview on the current state of the server and to optionally start syncronizations manually.
*  A well-documented HTTP-API to easily integrate the batyr into other applications and allow flexible triggering of synchroniszations. Furthermore the HTTP-API provides methods to integrate batyr in existing monitoring systems like Nagios.
* On-the-fly transformation of geometries to the spatial reference system of the database table. The required reference system is looked up in the PostGIS geometry_columns view/table and the transformation itself is performed by PostGIS.
* Internally batyr uses the OGR-library to access datasources. So batyr covers all vector formats supported by OGR and connecting to - for example - a WFS is possible. Additionally this allows using OGR Virtual Formats for extended configuration options.
* Synchronization jobs are internally queued and are handled in parallel using a configurable number of database connections. This takes care of a responsive HTTP-API as well as optimal usage of resources.

With these features it is possible to quickly integrate external geodata into your PostGIS database - without having to spend time creating custom code.

![Screenshot of the jobqueue of the webinterface](images/jobqueue_520b_0.png?raw=true)

For the complete manual see the included MANUAL.md file.

