#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// Creates an array of random floats. Each number has a value from 0 - 1
float *create_rand_nums(const int num_elements)
{
    float *rand_nums = (float *)malloc(sizeof(float) * num_elements);
    assert(rand_nums != NULL);
    for (int i = 0; i < num_elements; i++)
    {
        rand_nums[i] = (rand() / (float)RAND_MAX);
    }
    return rand_nums;
}

// Distance**2 between d-vectors pointed to by v1, v2.
float distance2(const float *v1, const float *v2, const int d)
{
    float dist = 0.0;
    for (int i = 0; i < d; i++)
    {
        float diff = v1[i] - v2[i];
        dist += diff * diff;
    }
    return dist;
}

// Assign a site to the correct cluster by computing its distances to
// each cluster centroid.
int assign_site(const float *site, float *centroids,
                const int k, const int d)
{
    int best_cluster = 0;
    float best_dist = distance2(site, centroids, d);
    float *centroid = centroids + d;
    for (int c = 1; c < k; c++, centroid += d)
    {
        float dist = distance2(site, centroid, d);
        if (dist < best_dist)
        {
            best_cluster = c;
            best_dist = dist;
        }
    }
    return best_cluster;
}

// Add a site (vector) into a sum of sites (vector).
void add_site(const float *site, float *sum, const int d)
{
    for (int i = 0; i < d; i++)
    {
        sum[i] += site[i];
    }
}

// Print the centroids one per line.
void print_centroids(float *centroids, const int k, const int d)
{
    float *p = centroids;
    printf("Centroids:\n");
    for (int i = 0; i < k; i++)
    {
        for (int j = 0; j < d; j++, p++)
        {
            printf("%f ", *p);
        }
        printf("\n");
    }
}

int main(int argc, char **argv)
{
    int sites_per_proc = atoi(argv[1]);
    int nprocs = 8;
    int k = atoi(argv[2]); // number of clusters.
    int d = atoi(argv[3]); // dimension of data.
    srand(31359);
    float *sums;
    assert(sums = malloc(k * d * sizeof(float)));
    // The number of sites assigned to each cluster by this process. k integers.
    int *counts;
    assert(counts = malloc(k * sizeof(int)));
    // The current centroids against which sites are being compared.
    // These are shipped to the process by the root process.
    float *centroids;
    assert(centroids = malloc(k * d * sizeof(float)));

    float *all_sites = NULL;
    int *labels;

    all_sites = create_rand_nums(d * sites_per_proc * nprocs);
    // Take the first k sites as the initial cluster centroids.
    for (int i = 0; i < k * d; i++)
    {
        centroids[i] = all_sites[i];
    }
    print_centroids(centroids, k, d);
    assert(labels = malloc(nprocs * sites_per_proc * sizeof(int)));

    float norm = 1.0;

    while (norm > 0.00001)
    { // While they've moved...
        for (int i = 0; i < k * d; i++)
            sums[i] = 0.0;
        for (int i = 0; i < k; i++)
            counts[i] = 0;

        // Find the closest centroid to each site and assign to cluster.
        float *site = all_sites;
        for (int i = 0; i < sites_per_proc * nprocs; i++, site += d)
        {
            int cluster = assign_site(site, centroids, k, d);
            // Record the assignment of the site to the cluster.
            counts[cluster]++;
            add_site(site, &sums[cluster * d], d);
        }

        for (int i = 0; i < k; i++)
        {
            for (int j = 0; j < d; j++)
            {
                int dij = d * i + j;
                sums[dij] /= counts[i];
            }
        }
        // Have the centroids changed much?
        norm = distance2(sums, centroids, d * k);
        printf("norm: %f\n", norm);
        // Copy new centroids from grand_sums into centroids.
        for (int i = 0; i < k * d; i++)
        {
            centroids[i] = sums[i];
        }
        print_centroids(centroids, k, d);
    }

    // Now centroids are fixed, so compute a final label for each site.
    float *site = all_sites;
    for (int i = 0; i < sites_per_proc * nprocs; i++, site += d)
    {
        labels[i] = assign_site(site, centroids, k, d);
    }
}
