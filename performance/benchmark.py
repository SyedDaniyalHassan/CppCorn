import asyncio
import aiohttp
import time
import argparse
import statistics
import sys
from collections import defaultdict

# Add simple colors for terminal output
class Colors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'

async def worker(session, url, duration, stats):
    end_time = time.time() + duration
    while time.time() < end_time:
        start_req = time.time()
        try:
            async with session.get(url) as response:
                await response.read()
                latency = (time.time() - start_req) * 1000 # ms
                stats['latencies'].append(latency)
                stats['requests'] += 1
                if response.status == 200:
                    stats['success'] += 1
                else:
                    stats['errors'] += 1
        except Exception as e:
            stats['errors'] += 1
            # Don't sleep on error, just retry tight loop
            pass

async def main():
    parser = argparse.ArgumentParser(description="HTTP Benchmark Tool")
    parser.add_argument("--url", default="http://localhost:8000/", help="Target URL")
    parser.add_argument("--clients", type=int, default=50, help="Number of concurrent clients")
    parser.add_argument("--time", type=int, default=10, help="Duration in seconds")
    args = parser.parse_args()

    print(f"{Colors.HEADER}Starting Benchmark...{Colors.ENDC}")
    print(f"Target: {Colors.BOLD}{args.url}{Colors.ENDC}")
    print(f"Clients: {args.clients}, Duration: {args.time}s")
    print("-" * 40)

    stats = defaultdict(int)
    stats['latencies'] = []

    # Create a single session for all workers (connection pooling)
    connector = aiohttp.TCPConnector(limit=args.clients)
    async with aiohttp.ClientSession(connector=connector) as session:
        tasks = []
        start_time = time.time()
        # Spawn workers
        for _ in range(args.clients):
            tasks.append(worker(session, args.url, args.time, stats))
        
        await asyncio.gather(*tasks)
        total_time = time.time() - start_time

    # Calculate results
    req_count = stats['requests']
    success_count = stats['success']
    error_count = stats['errors']
    latencies = stats['latencies']

    rps = req_count / total_time if total_time > 0 else 0
    
    if latencies:
        avg_lat = statistics.mean(latencies)
        median_lat = statistics.median(latencies)
        p95 = statistics.quantiles(latencies, n=20)[18]  # approx 95th percentile
        p99 = statistics.quantiles(latencies, n=100)[98] # approx 99th percentile
        min_lat = min(latencies)
        max_lat = max(latencies)
    else:
        avg_lat = median_lat = p95 = p99 = min_lat = max_lat = 0

    print(f"\n{Colors.OKGREEN}Benchmark Completed!{Colors.ENDC}\n")
    print(f"{Colors.BOLD}Results:{Colors.ENDC}")
    print(f"  Total Requests:    {req_count}")
    print(f"  Successful:        {success_count}")
    print(f"  Failed:            {error_count}")
    print(f"  Duration:          {total_time:.2f} s")
    print("-" * 40)
    print(f"  {Colors.OKBLUE}Requests Per Second: {rps:.2f} req/s{Colors.ENDC}")
    print("-" * 40)
    print(f"  Latency (ms):")
    print(f"    Avg:    {avg_lat:.2f}")
    print(f"    Min:    {min_lat:.2f}")
    print(f"    Max:    {max_lat:.2f}")
    print(f"    Median: {median_lat:.2f}")
    print(f"    P95:    {p95:.2f}")
    print(f"    P99:    {p99:.2f}")

if __name__ == "__main__":
    if sys.platform == 'win32':
        asyncio.set_event_loop_policy(asyncio.WindowsSelectorEventLoopPolicy())
    asyncio.run(main())
