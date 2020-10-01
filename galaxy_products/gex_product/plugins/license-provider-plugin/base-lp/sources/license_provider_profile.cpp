
/***** Routines to read profile strings --  by Joseph J. Graf ******/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#ifdef _WIN32
    #include <io.h>
#endif

#include "license_provider_profile.h"  // function prototypes in here

#if defined unix || __MACH__
    #include <unistd.h>
#endif

/*****************************************************************
* Function:     read_line()
* Arguments:    <FILE *> fp - a pointer to the file to be read from
*               <char *> bp - a pointer to the copy buffer
* Returns:      true if successful false otherwise
******************************************************************/
int read_line(FILE *fp, char *bp)
{   char c = '\0';
    int i = 0;

	// Read one line from the source file
    while( (c = getc(fp)) != '\n' )
    {   
		if( c == EOF )         /* return false on unexpected EOF */
            return(0);
        bp[i++] = c;
    }
    bp[i] = '\0';
    return(1);
}
/**************************************************************************
* Function:     get_private_profile_int()
* Arguments:    <const char *> section		- the name of the section to search for
*               <const char *> entry		- the name of the entry to find the value of
*               <int> def					- the default value in the event of a failed read
*               <const char *> file_name	- the name of the .ini file to read from
* Returns:      the value located at entry
***************************************************************************/
int get_private_profile_int(const char *section, 
    const char *entry, int def, const char *file_name)
{
    FILE *fp = fopen(file_name,"r");
    char buff[MAX_LINE_LENGTH+1]="";
    char *ep=0;
    char t_section[MAX_LINE_LENGTH+1]="";
    char value[6]="";
    int len = strlen(entry);
    int i=0;
    if( !fp ) 
		return(def);

    sprintf(t_section,"[%s]",section); /* Format the section name */
    /*  Move through file 1 line at a time until a section is matched or EOF */
    do
    {
        if( !read_line(fp,buff) )
        {   fclose(fp);
            //fflush(fp);       PYC, 02/09/2011
            return(def);
        }
    }
    while( strcmp(buff,t_section) );
    /* Now that the section has been found, find the entry.
     * Stop searching upon leaving the section's area. */
    do
    {   if( !read_line(fp,buff) || buff[0] == '\0' )
        {
            fclose(fp);
            //fflush(fp);       PYC, 02/09/2011
            return(def);
        }
    }
    while( strncmp(buff,entry,len) );
    ep = strrchr(buff,'=');    /* Parse out the equal sign */
    ep++;
    if( !strlen(ep) )          /* No setting? */
        return(def);
    /* Copy only numbers fail on characters */

    for(i = 0; isdigit(ep[i]); i++ )
        value[i] = ep[i];
    value[i] = '\0';
    fclose(fp);  
    //fflush(fp);       PYC, 02/09/2011
	// Check if entry exists, but is empty !...then return default value.
	if(*value == 0)
		return(def);

	/* Clean up and return the value */
    return(atoi(value));
}

/**************************************************************************
* Function:     get_private_profile_string()
* Arguments:    <const char *> section	- the name of the section to search for
*               <const char *> entry	- the name of the entry to find the value of
*               <cosnt char *> def		- default string in the event of a failed read
*               <char *> buffer			- a pointer to the buffer to copy into
*               <int> buffer_len		- the max number of characters to copy
*               <const char *> file_name - the name of the .ini file to read from
* Returns:      the number of characters copied into the supplied buffer
***************************************************************************/
int get_private_profile_string(const char *section, const char *entry, const char *def, 
    char *buffer, int buffer_len, const char *file_name)
{   FILE *fp;
    char buff[MAX_LINE_LENGTH+1];
    char *ep;
    char t_section[MAX_LINE_LENGTH+1];
    int len = strlen(entry);
	
	// if the file does not exist copy the default value to the destination buffer.
	fp = fopen(file_name,"r");
    if(fp == NULL) 
	{
		strcpy(buffer,def);
		return(strlen(def));
	}

    sprintf(t_section,"[%s]",section);    /* Format the section name */
    /*  Move through file 1 line at a time until a section is matched or EOF */
    do
    {   if( !read_line(fp,buff) )
        {   fclose(fp);
            //fflush(fp);       PYC, 02/09/2011
            strncpy(buffer,def,buffer_len);     
            return(strlen(buffer));
        }
    }
    while( strcmp(buff,t_section) );
    /* Now that the section has been found, find the entry.
     * Stop searching upon leaving the section's area. */
    do
    {   if( !read_line(fp,buff) || buff[0] == '\0' )
        {   fclose(fp);
            // fflush(fp);      PYC, 02/09/2011
            strncpy(buffer,def,buffer_len);     
            return(strlen(buffer));
        }
    }  while( strncmp(buff,entry,len) );
    ep = strrchr(buff,'=');    /* Parse out the equal sign */
    ep++;
    /* Copy up to buffer_len chars to buffer */
    strncpy(buffer,ep,buffer_len - 1);

    buffer[buffer_len] = '\0';
    fclose(fp); 
    //fflush(fp);   PYC, 02/09/2011
	// Check if entry exists, but is empty !...then return default value.
	if(*buffer == 0)
		strcpy(buffer,def);
	// Clean up and return the amount copied
    return(strlen(buffer));
}



/***************************************************************************
 * Function:    write_private_profile_string()
 * Arguments:   <const char *> section		- the name of the section to search for
 *              <const char *> entry		- the name of the entry to find the value of
 *              <const char *> buffer		- pointer to the buffer that holds the string
 *              <const char *> file_name	- the name of the .ini file to read from
 * Returns:     true if successful, otherwise false
 ***************************************************************************/
int write_private_profile_string(const char *section, 
    const char *entry, const char *buffer, const char *file_name)
{   
	FILE *rfp, *wfp;
    char tmp_name[MAX_LINE_LENGTH+1];
    char buff[MAX_LINE_LENGTH+1];
    char t_section[MAX_LINE_LENGTH+1];
    int len;

	// Build temporary name.
	strcpy(tmp_name,file_name);
	strcat(tmp_name,".tmp");
	// Format the section entry name: add brackets!
	sprintf(t_section,"[%s]",section);

	// Check if .ini file exists. if not, create it!
    rfp = fopen(file_name,"r");

	if(rfp == NULL)
    {   
		// Create .ini file
		wfp = fopen(file_name,"w");
		if(wfp == NULL)
			return(0);   
		// New .ini file created, add section + entry.
        fprintf(wfp,"%s\n",t_section);
        fprintf(wfp,"%s=%s\n",entry,buffer);
        fclose(wfp);
        // fflush(wfp);     PYC, 02/09/2011
        return(1);
    }

	// .INI file already exists, create .INI destinatyion one...
    wfp = fopen(tmp_name,"w");
	if(wfp == NULL)
    {   
		// Failed creating new .INI file...
		fclose(rfp);
        // fflush(rfp);     PYC, 02/09/2011
        return(0);
    }

	// Copy original .INI file to destination .INI file unti section is found or EOF
    do
    {   if( fgets(buff,MAX_LINE_LENGTH,rfp) == NULL)
        {   
			// End of file: section not found, so add it.
            fprintf(wfp,"\n%s\n",t_section);
            fprintf(wfp,"%s=%s\n",entry,buffer);
            // Clean up and rename 
            fclose(rfp);
            // fflush(rfp);     PYC, 02/09/2011
            fclose(wfp);
            //fflush(wfp);      PYC, 02/09/2011
            unlink(file_name);
            rename(tmp_name,file_name);
            return(1);
        }
        fputs(buff,wfp);
    } while( strstr(buff,t_section) == NULL );

	// Arrive here when section is found.
	// So lets find the entry
    while( 1 )
    {   
		if(fgets(buff,MAX_LINE_LENGTH,rfp) == NULL)
        {   
			// EOF without an entry so make one 
            fprintf(wfp,"%s=%s\n",entry,buffer);
            /* Clean up and rename */
            fclose(rfp);
            //fflush(rfp);      PYC, 02/09/2011
            fclose(wfp);
            //fflush(wfp);      PYC, 02/09/2011
            unlink(file_name);
            rename(tmp_name,file_name);
            return(1);

        }
		// Check if this line is the field we look for...or end of section
	    len = strlen(entry);
        if( !strncmp(buff,entry,len) || buff[0] == '\0' || buff[0] == '\n' )
            break;	// yes
		// no: this is not the field we look for, so copy it to new .INI
        fputs(buff,wfp);
    }

	// Arrive here when section+field found !
    if( buff[0] == '\0' || buff[0] == '\n' )
    {   
		// We reached the end of the section: add our new field
		fprintf(wfp,"%s=%s\n",entry,buffer);

		// Copy the rest of the file.
        do
        {
			fputs(buff,wfp);
        } while(fgets(buff,MAX_LINE_LENGTH,rfp) != NULL);
    }
    else
    {   
		// We have found the section+field: copy the new field
		fprintf(wfp,"%s=%s\n",entry,buffer);
		// Append the rest of the .INI file
        while(fgets(buff,MAX_LINE_LENGTH,rfp) != NULL)
        {
             fputs(buff,wfp);
        }
    }
    // Clean up and rename 
    fclose(wfp);
    // fflush(wfp);     PYC, 02/09/2011
    fclose(rfp);
    // fflush(rfp);     PYC, 02/09/2011
    unlink(file_name);
    rename(tmp_name,file_name);
    return(1);
}
