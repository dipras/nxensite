#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const char defaultLoc[] = "/etc/nginx";

struct FilteredFile {
  char name[32];
  char path[128];
  int enabled;
};

int isFileExists(const char *path);
int listFiles(const char *path, char ***files);
int addFilteredFile(struct FilteredFile file,
                    struct FilteredFile **filteredFiles, int count);

int main() {
  if (!isFileExists(defaultLoc)) {
    perror("Folder not exists. Please confirm that nginx is installed! \n");
    return 1;
  }

  char **files = NULL;
  char **linkFiles = NULL;
  struct FilteredFile *filteredFiles = NULL;
  char temp[128];

  // List files in sites-available
  snprintf(temp, sizeof(temp), "%s%s", defaultLoc, "/sites-available");
  if (!listFiles(temp, &files)) {
    return 1;
  }

  // List files in sites-enabled
  memset(temp, 0, sizeof(temp));
  snprintf(temp, sizeof(temp), "%s%s", defaultLoc, "/sites-enabled");
  if (!listFiles(temp, &linkFiles)) {
    return 1;
  }

  int count = 0;
  while (files[count] != NULL) {
    memset(temp, 0, sizeof(temp));
    snprintf(temp, sizeof(temp), "%s/sites-enabled/%s", defaultLoc,
             files[count]);
    int enabled = isFileExists(temp) ? 1 : 0;

    struct FilteredFile file;
    strcpy(file.path, temp);
    strcpy(file.name, files[count]);
    file.enabled = enabled;
    if (!addFilteredFile(file, &filteredFiles, count)) {
      return 1;
    }

    count++;
  }

  // Cleanup
  for (int i = 0; files[i] != NULL; i++) {
    free(files[i]);
  }
  free(files);

  for (int i = 0; linkFiles[i] != NULL; i++) {
    free(linkFiles[i]);
  }
  free(linkFiles);

  printf("Please choose one of config by input the number. If the config is "
         "enabled, then it will be disabled. If the config is disabled, then "
         "it will be enabled! \n");

  for (int i = 0; i < count; i++) {
    struct FilteredFile file = filteredFiles[i];
    printf("%i. %s : %s \n", (i + 1), file.name,
           file.enabled ? "Enabled" : "Disabled");
  }

  int answer;
  printf("Enter the number: ");
  scanf("%i", &answer);
  if (answer > count || answer <= 0) {
    printf("The number you input is invalid! \n");
    return 1;
  }

  struct FilteredFile choseFile = filteredFiles[answer - 1];
  if (choseFile.enabled) {
    unlink(choseFile.path);
    printf("Your site have been disabled \n");
  } else {
    memset(temp, 0, sizeof(temp));
    snprintf(temp, sizeof(temp), "%s/sites-available/%s", defaultLoc,
             choseFile.name);
    char temp2[128];
    snprintf(temp2, sizeof(temp2), "%s/sites-enabled/%s", defaultLoc,
             choseFile.name);
    int err = symlink(temp, temp2);
    if (err != 0) {
      perror("Error when symlink: ");
      return 1;
    }
    printf("Symlink is success, your site is enabled. Restart your nginx "
           "server! \n");
  }

  return 0;
}

int isFileExists(const char *path) {
  FILE *fptr = fopen(path, "r");
  if (fptr == NULL) {
    return 0;
  }
  fclose(fptr);
  return 1;
}

int listFiles(const char *path, char ***files) {
  struct dirent *entry;
  DIR *dp = opendir(path);

  if (dp == NULL) {
    perror("Error when opening directory");
    return 0;
  }

  int count = 0;
  *files = NULL;

  while ((entry = readdir(dp))) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
      continue;
    }

    *files = realloc(*files, (count + 1) * sizeof(char *));
    if (*files == NULL) {
      perror("Error reallocating memory");
      closedir(dp);
      return 0;
    }

    (*files)[count] = malloc(strlen(entry->d_name) + 1);
    if ((*files)[count] == NULL) {
      perror("Error allocating memory");
      closedir(dp);
      return 0;
    }
    strcpy((*files)[count], entry->d_name);
    count++;
  }

  *files = realloc(*files, (count + 1) * sizeof(char *));
  (*files)[count] = NULL;
  closedir(dp);
  return 1;
}

int addFilteredFile(struct FilteredFile file,
                    struct FilteredFile **filteredFiles, int count) {
  *filteredFiles =
      realloc(*filteredFiles, (count + 1) * sizeof(struct FilteredFile));
  if (*filteredFiles == NULL) {
    perror("Error when reallocating memory");
    return 0;
  }
  (*filteredFiles)[count] = file;
  return 1;
}
