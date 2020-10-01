#include "stdafx.h"
// Disable warning messages 4996.
#pragma warning( disable : 4996 )

/***** Routines to read profile strings --  by Joseph J. Graf ******/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "profile.h"   /* function prototypes in here */

#ifdef unix
#include <unistd.h>
#endif


/*****************************************************************
* Function:     read_line()
* Arguments:    <FILE *> fp - a pointer to the file to be read from
*               <char *> bp - a pointer to the copy buffer
* Returns:      TRUE if successful FALSE otherwise
******************************************************************/
int read_line(FILE *fp, char *bp)
{   char c = '\0';
    int i = 0;
    
	// Read one line from the source file
    while( (c = getc(fp)) != '\n' )
    {   
		if( c == EOF )         /* return FALSE on unexpected EOF */
            return(0);
        bp[i++] = c;
    }
    bp[i] = '\0';
    return(1);
}
/**************************************************************************
* Function:     get_private_profile_int()
* Arguments:    <char *> section - the name of the section to search for
*               <char *> entry - the name of the entry to find the value of
*               <int> def - the default value in the event of a failed read
*               <char *> file_name - the name of the .ini file to read from
* Returns:      the value located at entry
***************************************************************************/
int get_private_profile_int(char *section, 
    char *entry, int def, char *file_name)
{   FILE *fp = fopen(file_name,"r");
    char buff[MAX_LINE_LENGTH+1];
    char *ep;
    char t_section[MAX_LINE_LENGTH+1];
    char value[6];
    int len = strlen(entry);
    int i;
    if( !fp ) 
		return(def);

    sprintf(t_section,"[%s]",section); /* Format the section name */
    /*  Move through file 1 line at a time until a section is matched or EOF */
    do
    {   if( !read_line(fp,buff) )
        {   fclose(fp);
			fflush(fp);
            return(def);
        }
    } while( strcmp(buff,t_section) );
    /* Now that the section has been found, find the entry.
     * Stop searching upon leaving the section's area. */
    do
    {   if( !read_line(fp,buff) || buff[0] == '\0' )
        {   fclose(fp);
			fflush(fp);
            return(def);
        }
    }  while( strncmp(buff,entry,len) );
    ep = strrchr(buff,'=');    /* Parse out the equal sign */
    ep++;
    if( !strlen(ep) )          /* No setting? */
        return(def);
    /* Copy only numbers fail on characters */

    for(i = 0; isdigit(ep[i]); i++ )
        value[i] = ep[i];
    value[i] = '\0';
    fclose(fp);  
	fflush(fp);
	// Check if entry exists, but is empty !...then return default value.
	if(*value == 0)
		return(def);

	/* Clean up and return the value */
    return(atoi(value));
}
/**************************************************************************
* Function:     get_private_profile_string()
* Arguments:    <char *> section - the name of the section to search for
*               <char *> entry - the name of the entry to find the value of
*               <char *> def - default string in the event of a failed read
*               <char *> buffer - a pointer to the buffer to copy into
*               <int> buffer_len - the max number of characters to copy
*               <char *> file_name - the name of the .ini file to read from
* Returns:      the number of characters copied into the supplied buffer
***************************************************************************/
int get_private_profile_string(char *section, char *entry, char *def, 
    char *buffer, int buffer_len, char *file_name)
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
			fflush(fp);
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
			fflush(fp);
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
	fflush(fp);
	// Check if entry exists, but is empty !...then return default value.
	if(*buffer == 0)
		strcpy(buffer,def);
	// Clean up and return the amount copied
    return(strlen(buffer));
}
/***************************************************************************
 * Function:    write_private_profile_string()
 * Arguments:   <char *> section - the name of the section to search for
 *              <char *> entry - the name of the entry to find the value of
 *              <char *> buffer - pointer to the buffer that holds the string
 *              <char *> file_name - the name of the .ini file to read from
 * Returns:     TRUE if successful, otherwise FALSE
 ***************************************************************************/
int write_private_profile_string(char *section, 
    char *entry, char *buffer, char *file_name)

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
		fflush(wfp);
        return(1);
    }

	// .INI file already exists, create .INI destinatyion one...
    wfp = fopen(tmp_name,"w");
	if(wfp == NULL)
    {   
		// Failed creating new .INI file...
		fclose(rfp);
		fflush(rfp);
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
			fflush(rfp);
            fclose(wfp);
			fflush(wfp);
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
			fflush(rfp);
            fclose(wfp);
			fflush(wfp);
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
	fflush(wfp);
    fclose(rfp);
	fflush(rfp);
    unlink(file_name);
    rename(tmp_name,file_name);
    return(1);
}
/***************************************************************************
 * Added by S. Perry
 * Function:    delete_private_profile_section()
 * Arguments:   <char *> section - the name of the section to delete      
 *              <char *> file_name - the name of the .ini file to read from
 * Returns:     TRUE if successful, otherwise FALSE
 ***************************************************************************/
int delete_private_profile_section(char *section, char *file_name)
{   
	FILE *rfp, *wfp;
    char tmp_name[MAX_LINE_LENGTH+1];
    char buff[MAX_LINE_LENGTH+1];
    char t_section[MAX_LINE_LENGTH+1];
	int isEOS;  // End of section flag

	// Build temporary name.
	strcpy(tmp_name,file_name);
	strcat(tmp_name,".tmp");
	// Format the section entry name: add brackets!
	sprintf(t_section,"[%s]",section);

	// Check if .ini file exists. if not, return!
    rfp = fopen(file_name,"r");

	if(rfp == NULL) 
        return(1);

	// .INI file exists, create temp .INI destination
    wfp = fopen(tmp_name,"w");
	if(wfp == NULL)
    {   
		// Failed creating new .INI file...
		fclose(rfp);
		fflush(rfp);
        return(0);
    }
	 
	// Copy original .INI file to destination .INI file until section or EOF is found 
	isEOS = 0;
	while ( 1 )
    {   
		if( fgets(buff,MAX_LINE_LENGTH,rfp) == NULL) 
        {   
			// End of file - section not found
			if (isEOS == 1)
				fprintf(wfp,"\n");
			fclose(rfp);
			fflush(rfp);
			fclose(wfp);
			fflush(wfp);
			remove(tmp_name);
            return(1);
        }
        // Copy each record until section is found
		if (strstr(buff,t_section) == NULL)
		// Not target section
		{
			if (isEOS == 1)								// Write end of section
			{					
				fprintf(wfp,"\n");
				isEOS = 0;
			}
			if( buff[0] == '\0' || buff[0] == '\n' )	
				isEOS = 1;								// End of section found
			else		
				fputs(buff,wfp);						// End of section not found
		}
		// Is target section
		else		
			break;	// Section found don't copy the record
    }
	
	// Section to delete
	while( 1 )		// Look for end of section without copying records
    {   
		if(fgets(buff,MAX_LINE_LENGTH,rfp) == NULL)  
        {   
			// EOF reached without other sections to handle, so clean up and rename        
			fclose(rfp);
			fflush(rfp);
            fclose(wfp);
			fflush(wfp);
            unlink(file_name);
            rename(tmp_name,file_name);
			return(1);
        }
		// Check if this line is the end of section
        if( buff[0] == '\0' || buff[0] == '\n' )
            break;	// Yes
    }	
	
    if (isEOS == 1)									// Write end of last kept section
		fprintf(wfp,"\n");
	while(fgets(buff,MAX_LINE_LENGTH,rfp) != NULL)	// Copy the rest of the file
    {
		fputs(buff,wfp);
    }
   
    // Clean up and rename 
    fclose(wfp);
	fflush(wfp);
    fclose(rfp);
	fflush(rfp);
    unlink(file_name);
    rename(tmp_name,file_name);
    return(1);
}
