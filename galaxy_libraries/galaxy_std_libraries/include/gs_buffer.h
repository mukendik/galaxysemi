/******************************************************************************!
 * \file gs_buffer.h
 * \brief Generic buffer which is automaticaly resized when needed
 ******************************************************************************/
#ifndef GS_BUFFER_H
#define GS_BUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

struct GsBuffer {
    /*!
     * \var capacity
     * \brief Allocated size
     */
    unsigned int capacity;
    /*!
     * \var ptr
     * \brief Start of the buffer
     */
    char* ptr;
    /*!
     * \var size
     * \brief Size of the datas in the buffer
     */
    size_t size;
};

/*!
 * \fn gs_buffer_new
 * \brief Create a descriptor
 * \return GsBuffer descriptor
 */
struct GsBuffer* gs_buffer_new();
/*!
 * \fn gs_buffer_init
 * \brief Initialize a descriptor
 * \return GsBuffer descriptor
 */
struct GsBuffer* gs_buffer_init(struct GsBuffer* b);
/*!
 * \fn gs_buffer_quit
 * \brief The oposite of init, do not forget to free the descriptor
 * \return void
 */
void gs_buffer_quit(struct GsBuffer* b);
/*!
 * \fn gs_buffer_add
 * \brief Add a formatted string to the buffer, like printf
 *        When the string is not well known, use:
 *        gs_buffer_add(buff, "%s", str), not: gs_buffer_add(buff, str)
 * \return void
 */
void gs_buffer_add(struct GsBuffer* b, const char* fmt, ...);
/*!
 * \fn gs_buffer_get
 * \brief Get the finalized string
 * \return String
 */
char* gs_buffer_get(struct GsBuffer* b);

#ifdef __cplusplus
}
#endif

#endif
