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

#ifndef MARKETPLACEAPP_H_
#define MARKETPLACEAPP_H_

#include "PoolSQL.h"
#include "ObjectCollection.h"
#include "MarketPlaceAppTemplate.h"

/**
 *  The MarketPlaceApp class. It represents an abstract application for
 *  OpenNebula objects (Image, VM Templates or Flows)
 */
class MarketPlaceApp : public PoolObjectSQL
{
public:
    enum MarketPlaceAppState
    {
        INIT      = 0, /** < Initialization state */
        READY     = 1, /** < Ready to use */
        LOCKED    = 2, /** < Operation in process */
        ERROR     = 3, /** < Error state the operation failed*/
    };

    /**
     * Returns the string representation of an MarketplaceApp State
     * @param state The state
     * @return the string representation
     */
    static string state_to_str(MarketPlaceAppState state)
    {
        switch(state)
        {
            case INIT:   return "INIT";   break;
            case READY:  return "READY";  break;
            case LOCKED: return "LOCKED"; break;
            case ERROR:  return "ERROR";  break;
            default:     return "";
        }
    };


    /**
     * Function to print the MarketPlaceApp object into a string in XML format
     *  @param xml the resulting XML string
     *  @return a reference to the generated string
     */
    string& to_xml(std::string& xml) const;

    /**
     *  Rebuilds the object from an xml formatted string
     *    @param xml_str The xml-formatted string
     *
     *    @return 0 on success, -1 otherwise
     */
    int from_xml(const std::string &xml_str);

private:

    friend class MarketPlaceAppPool;

    // *************************************************************************
    // MarketPlaceApp Attributes
    // *************************************************************************
    /**
     *  Size of this app
     */
    std::string date;

    /**
     *  Source URL for the marketplace app
     */
    std::string source;

    /**
     *  Source URL for the marketplace app
     */
    std::string checksum;

    /**
     *  Size of this app
     */
    long long size_mb;

    /**
     *  Description of the App
     */
    std::string description;

    /**
     *  User sharing the app
     */
    std::string publisher;

    /**
     *  Version of the app
     */
    std::string version;

    /**
     * App template to import it
     */
    std::string apptemplate64;

    /**
     *  Marketplace ID that holds this app
     */
    int mp_id;

    /**
     *  Marketplace name
     */
    std::string mp_name;

    /**
     *  Marketplace App state
     */
    MarketPlaceAppState state;

    /**
     *  Origin of this App
     */
    std::string origin;

    // *************************************************************************
    // Constructor
    // *************************************************************************
    MarketPlaceApp(
            int                     uid,
            int                     gid,
            const std::string&      uname,
            const std::string&      gname,
            int                     umask,
            MarketPlaceAppTemplate* app_template);

    virtual ~MarketPlaceApp();

    // *************************************************************************
    // DataBase implementation (Private)
    // *************************************************************************

    static const char * db_names;

    static const char * db_bootstrap;

    static const char * table;

    /**
     *  Execute an INSERT or REPLACE Sql query.
     *    @param db The SQL DB
     *    @param replace Execute an INSERT or a REPLACE
     *    @param error_str Returns the error reason, if any
     *    @return 0 one success
     */
    int insert_replace(SqlDB *db, bool replace, std::string& error_str);

    /**
     *  Bootstraps the database table(s) associated to the MarketPlace
     *    @return 0 on success
     */
    static int bootstrap(SqlDB * db)
    {
        ostringstream oss(db_bootstrap);

        return db->exec(oss);
    };

    /**
     *  Writes the MarketPlace in the database.
     *    @param db pointer to the db
     *    @return 0 on success
     */
    int insert(SqlDB *db, std::string& error_str);

    /**
     *  Writes/updates the MarketPlace's data fields in the database.
     *    @param db pointer to the db
     *    @return 0 on success
     */
    int update(SqlDB *db)
    {
        std::string error_str;
        return insert_replace(db, true, error_str);
    }

    /**
     * Child classes can process the new template set with replace_template or
     * append_template with this method
     *    @param error string describing the error if any
     *    @return 0 on success
     */
    int post_update_template(std::string& error);
};

#endif /*MARKETPLACEAPP_H*/
