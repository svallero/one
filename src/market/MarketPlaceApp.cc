/* ------------------------------------------------------------------------ */
/* Copyright 2002-2015, OpenNebula Project, OpenNebula Systems              */
/*                                                                          */
/* Licensed under the Apache License, Version 2.0 (the "License"); you may  */
/* not use this file except in compliance with the License. You may obtain  */
/* a copy of the License at                                                 */
/*                                                                          */
/* http://www.apache.org/licenses/LICENSE-2.0                               */
/*                                                                          */
/* Unless required by applicable law or agreed to in writing, software      */
/* distributed under the License is distributed on an "AS IS" BASIS,        */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. */
/* See the License for the specific language governing permissions and      */
/* limitations under the License.                                           */
/* -------------------------------------------------------------------------*/

#include <sstream>
#include "MarketPlaceApp.h"
#include "NebulaLog.h"
#include "NebulaUtil.h"

/* ************************************************************************ */
/* MarketPlaceApp:: Database Definition                                     */
/* ************************************************************************ */

const char * MarketPlaceApp::table = "marketplaceapp_pool";

const char * MarketPlaceApp::db_names =
        "oid, name, body, uid, gid, owner_u, group_u, other_u";

const char * MarketPlaceApp::db_bootstrap =
    "CREATE TABLE IF NOT EXISTS marketplaceapp_pool (oid INTEGER PRIMARY KEY, "
    "name VARCHAR(128), body MEDIUMTEXT, uid INTEGER, gid INTEGER, "
    "owner_u INTEGER, group_u INTEGER, other_u INTEGER, UNIQUE(name,uid))";

/* ************************************************************************ */
/* MarketPlaceApp:: Constructor / Destructor                                */
/* ************************************************************************ */

MarketPlaceApp::MarketPlaceApp(
    int                      uid,
    int                      gid,
    const std::string&       uname,
    const std::string&       gname,
    int                      umask,
    MarketPlaceAppTemplate * app_template):
        PoolObjectSQL(-1, MARKETPLACEAPP, "", uid, gid, uname, gname, table),
        source(""),
        checksum(""),
        size_mb(0),
        description(""),
        publisher(""),
        version(""),
        apptemplate64(""),
        mp_id(-1),
        mp_name(""),
        state(INIT)
{
    if (app_template != 0)
    {
        obj_template = app_template;
    }
    else
    {
        obj_template = new MarketPlaceAppTemplate;
    }

    set_umask(umask);
};

MarketPlaceApp::~MarketPlaceApp()
{
    delete obj_template;
};

/* ************************************************************************ */
/* MartketPlaceApp:: Database Access Functions                              */
/* ************************************************************************ */

int MarketPlaceApp::insert(SqlDB *db, string& error_str)
{
    std::ostringstream oss;

    std::string attr;

    char        str[26];
    time_t      the_time = time(NULL);

    // -------------------------------------------------------------------------
    // Check default marketplace app attributes
    // -------------------------------------------------------------------------

	//MarketPlaceAppPool::allocate checks NAME
    erase_template_attribute("NAME", name);

    //Atrributes updated after export
    remove_template_attribute("ORIGIN");
    remove_template_attribute("SOURCE");
    remove_template_attribute("SIZE");
    remove_template_attribute("CHECKSUM");

    ctime_r(&(the_time), str);
    date = str;

    //Known attributes
    //DESCRIPTION
    //APPTEMPLATE64
    //PUBLISHER
    //VERSION

    get_template_attribute("DESCRIPTION", description);

    get_template_attribute("APPTEMPLATE64", apptemplate64);

    get_template_attribute("PUBLISHER", attr);

    if (attr.empty())
    {
        publisher = uname;
    }

    get_template_attribute("VERSION", attr);

    if (!attr.empty())
    {
        version = attr;
    }

    state = LOCKED;

    //--------------------------------------------------------------------------

    return insert_replace(db, false, error_str);
}

/* --------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------- */

int MarketPlaceApp::insert_replace(SqlDB *db, bool replace, string& error_str)
{
    std::ostringstream   oss;

    int rc;
    std::string xml_body;

    char * sql_name;
    char * sql_xml;

    sql_name = db->escape_str(name.c_str());

    if ( sql_name == 0 )
    {
        goto error_name;
    }

    sql_xml = db->escape_str(to_xml(xml_body).c_str());

    if ( sql_xml == 0 )
    {
        goto error_body;
    }

    if ( validate_xml(sql_xml) != 0 )
    {
        goto error_xml;
    }

    if ( replace )
    {
        oss << "REPLACE";
    }
    else
    {
        oss << "INSERT";
    }

    // Construct the SQL statement to Insert or Replace
    oss <<" INTO "<<table <<" ("<< db_names <<") VALUES ("
        <<          oid                 << ","
        << "'" <<   sql_name            << "',"
        << "'" <<   sql_xml             << "',"
        <<          uid                 << ","
        <<          gid                 << ","
        <<          owner_u             << ","
        <<          group_u             << ","
        <<          other_u             << ")";

    rc = db->exec(oss);

    db->free_str(sql_name);
    db->free_str(sql_xml);

    return rc;

error_xml:
    db->free_str(sql_name);
    db->free_str(sql_xml);

    error_str = "Error transforming the marketplace app to XML.";

    goto error_common;

error_body:
    db->free_str(sql_name);
    goto error_generic;

error_name:
    goto error_generic;

error_generic:
    error_str = "Error inserting marketplace app in DB.";
error_common:
    return -1;
}

/* --------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------- */

/* *************************************************************************** */
/* MartketPlaceApp :: Template Functions                                          */
/* *************************************************************************** */
std::string& MarketPlaceApp::to_xml(std::string& xml) const
{
	std::ostringstream oss;
	std::string        template_xml;
	std::string        perm_str;

    oss << "<MARKETPLACEAPP>"
			"<ID>"             << oid           << "</ID>"  <<
			"<UID>"            << uid           << "</UID>" <<
			"<GID>"            << gid           << "</GID>" <<
			"<UNAME>"          << uname         << "</UNAME>" <<
			"<GNAME>"          << gname         << "</GNAME>" <<
			"<DATE>"           << date          << "</DATE>" <<
			"<NAME>"           << name          << "</NAME>" <<
            "<ORIGIN>"         << origin        << "</ORIGIN>" <<
            "<SOURCE>"         << source        << "</SOURCE>" <<
            "<CHECKSUM>"       << checksum      << "</CHECKSUM>" <<
            "<SIZE>"           << size_mb       << "</SIZE>" <<
            "<DESCRIPTION>"    << description   << "</DESCRIPTION>" <<
            "<PUBLISHER>"      << publisher     << "</PUBLISHER>" <<
            "<VERSION>"        << version       << "</VERSION>" <<
            "<APPTEMPLATE64>"  << apptemplate64 << "</APPTEMPLATE64>" <<
            "<MARKETPLACE_ID>" << mp_id        << "</MARKETPLACE_ID>" <<
            "<MARKETPLACE>"    << mp_name       << "</MARKETPLACE>" <<
            "<STATE>"          << state         << "</STATE>" <<
			perms_to_xml(perm_str) <<
			obj_template->to_xml(template_xml) <<
        "</MARKETPLACEAPP>";

    xml = oss.str();

    return xml;
}

/* --------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------- */

int MarketPlaceApp::from_xml(const std::string &xml_str)
{
    int rc = 0;
    int istate;
    std::vector<xmlNodePtr> content;

    // Initialize the internal XML object
    update_from_str(xml_str);

    // ----- MARKETPLACEAPP base attributes -----
    rc += xpath(oid,          "/MARKETPLACEAPP/ID", -1);
    rc += xpath(uid,          "/MARKETPLACEAPP/UID", -1);
    rc += xpath(gid,          "/MARKETPLACEAPP/GID", -1);
    rc += xpath(uname,        "/MARKETPLACEAPP/UNAME", "not_found");
    rc += xpath(gname,        "/MARKETPLACEAPP/GNAME", "not_found");
    rc += xpath(name,         "/MARKETPLACEAPP/NAME", "not_found");
    rc += xpath(date,         "/MARKETPLACEAPP/DATE", "not_found");
    rc += xpath(source,       "/MARKETPLACEAPP/SOURCE","not_found");
    rc += xpath(origin,       "/MARKETPLACEAPP/ORIGIN","not_found");
    rc += xpath(istate,       "/MARKETPLACEAPP/STATE", -1);
    rc += xpath(description,  "/MARKETPLACEAPP/DESCRIPTION", "not_found");
    rc += xpath(size_mb,      "/MARKETPLACEAPP/SIZE", -1);
    rc += xpath(version,      "/MARKETPLACEAPP/VERSION", "not_found");
    rc += xpath(checksum,     "/MARKETPLACEAPP/CHECKSUM", "not_found");
    rc += xpath(publisher,    "/MARKETPLACEAPP/PUBLISHER", "not_found");
    rc += xpath(apptemplate64,"/MARKETPLACEAPP/APPTEMPLATE64", "not_found");
    rc += xpath(mp_name,      "/MARKETPLACEAPP/MARKETPLACE", "not_found");
    rc += xpath(mp_id,        "/MARKETPLACEAPP/MARKETPLACE_ID", -1);

    state = static_cast<MarketPlaceAppState>(istate);

	// ----- Permissions -----
    rc += perms_from_xml();

	// ----- TEMPLATE -----
    ObjectXML::get_nodes("/MARKETPLACEAPP/TEMPLATE", content);

    if (content.empty())
    {
        return -1;
    }

    rc += obj_template->from_xml_node(content[0]);

    ObjectXML::free_nodes(content);

    if (rc != 0)
    {
        return -1;
    }

    return 0;
}

/* --------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------- */

int MarketPlaceApp::post_update_template(string& error)
{
	std::string n_description;
	std::string n_apptemplate64;
	std::string n_publisher;
	std::string n_version;

	// -------------------------------------------------------------------------
    // Update well known attributes
    // -------------------------------------------------------------------------
    get_template_attribute("DESCRIPTION",   n_description);
    get_template_attribute("APPTEMPLATE64", n_apptemplate64);
    get_template_attribute("PUBLISHER",     n_publisher);
    get_template_attribute("VERSION",       n_version);

    description   = n_description;
    apptemplate64 = n_apptemplate64;
    publisher     = n_publisher;
    version       = n_version;

	return 0;
}
