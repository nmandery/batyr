--
-- PostgreSQL database dump
--

SET statement_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = off;
SET check_function_bodies = false;
SET client_min_messages = warning;
SET escape_string_warning = off;

--
-- Name: test; Type: SCHEMA; Schema: -; Owner: batyr
--

CREATE SCHEMA test;


ALTER SCHEMA test OWNER TO batyr;

SET search_path = test, pg_catalog;

SET default_tablespace = '';

SET default_with_oids = false;

--
-- Name: africa; Type: TABLE; Schema: test; Owner: batyr; Tablespace: 
--

CREATE TABLE africa (
    gid integer NOT NULL,
    id character varying(4),
    code character varying(4),
    country character varying(35),
    the_geom public.geometry,
    CONSTRAINT enforce_dims_the_geom CHECK ((public.st_ndims(the_geom) = 2)),
    CONSTRAINT enforce_geotype_the_geom CHECK (((public.geometrytype(the_geom) = 'MULTIPOLYGON'::text) OR (the_geom IS NULL))),
    CONSTRAINT enforce_srid_the_geom CHECK ((public.st_srid(the_geom) = (-1)))
);


ALTER TABLE test.africa OWNER TO batyr;

--
-- Name: africa_gid_seq; Type: SEQUENCE; Schema: test; Owner: batyr
--

CREATE SEQUENCE africa_gid_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE test.africa_gid_seq OWNER TO batyr;

--
-- Name: africa_gid_seq; Type: SEQUENCE OWNED BY; Schema: test; Owner: batyr
--

ALTER SEQUENCE africa_gid_seq OWNED BY africa.gid;


--
-- Name: gid; Type: DEFAULT; Schema: test; Owner: batyr
--

ALTER TABLE ONLY africa ALTER COLUMN gid SET DEFAULT nextval('africa_gid_seq'::regclass);


--
-- Name: africa_pkey; Type: CONSTRAINT; Schema: test; Owner: batyr; Tablespace: 
--

ALTER TABLE ONLY africa
    ADD CONSTRAINT africa_pkey PRIMARY KEY (gid);


--
-- PostgreSQL database dump complete
--

