int writen(int fd, const void *vptr, int n)
{
	int		nleft, nwritten;
	const char	*ptr;

	ptr = vptr;	/* can't do pointer arithmetic on void* */
	nleft = n;
	while (nleft > 0) {
		if ( (nwritten = write(fd, ptr, nleft)) <= 0)
			return(nwritten);		/* error */

		nleft -= nwritten;
		ptr   += nwritten;
	}
	return(n);
}
