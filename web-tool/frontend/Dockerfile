# frontend/Dockerfile
FROM node:18-alpine

# Cài đặt system dependencies
RUN apk add --no-cache libc6-compat

# Set working directory
WORKDIR /app

# Copy package files
COPY package.json package-lock.json* ./

# Cài đặt dependencies
RUN npm ci || npm install

# Copy source code
COPY . .

# Expose port
EXPOSE 3000

# Start dev server
CMD ["npm", "run", "dev", "--", "--host", "0.0.0.0", "--port", "3000"]